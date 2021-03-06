/*
 *
 *  Copyright (c) 2018
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "settings.h"
#include "systemsignalhandler.h"
#include <memory>

#ifdef Q_OS_LINUX

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>

#include <QSocketNotifier>

static int sighupFd[ 2 ] ;
static int sigtermFd[ 2 ] ;

static void setup_unix_signal_handlers()
{
	struct sigaction hup ;
	struct sigaction term ;

	hup.sa_handler = []( int q ){

		Q_UNUSED( q ) ;

		char a = 1 ;

		::write( sighupFd[ 0 ],&a,sizeof( a ) ) ;
	} ;

	sigemptyset( &hup.sa_mask ) ;
	hup.sa_flags = 0 ;
	hup.sa_flags |= SA_RESTART ;

	sigaction( SIGHUP,&hup,nullptr ) ;

	term.sa_handler = []( int q ){

		Q_UNUSED( q ) ;

		char a = 1 ;

		::write( sigtermFd[ 0 ],&a,sizeof( a ) ) ;
	} ;

	sigemptyset( &term.sa_mask ) ;
	term.sa_flags |= SA_RESTART ;

	sigaction( SIGTERM,&term,nullptr ) ;
}

systemSignalHandler::systemSignalHandler( QObject * parent,std::function< void( signal ) > function ) :
	m_parent( parent ),m_function( std::move( function ) )
{
}

void systemSignalHandler::listen()
{
	if( !settings::instance().unMountVolumesOnLogout() ){

		return ;
	}

	setup_unix_signal_handlers() ;

	::socketpair( AF_UNIX,SOCK_STREAM,0,sighupFd ) ;

	::socketpair(AF_UNIX,SOCK_STREAM,0,sigtermFd ) ;

	auto snHup = new QSocketNotifier( sighupFd[ 1 ],QSocketNotifier::Read,m_parent ) ;

	QObject::connect( snHup,&QSocketNotifier::activated,[ snHup,this ]( int ){
#if 1
		snHup->setEnabled( false ) ;

		m_function( signal::hup ) ;
#else
		snHup->setEnabled( false ) ;

		char tmp ;

		::read( sighupFd[ 1 ],&tmp,sizeof( tmp ) ) ;

		m_function( signal::hup ) ;

		snHup->setEnabled( true ) ;
#endif
	} ) ;

	auto snTerm = new QSocketNotifier( sigtermFd[ 1 ],QSocketNotifier::Read,m_parent ) ;

	QObject::connect( snTerm,&QSocketNotifier::activated,[ snTerm,this ]( int ){
#if 1
		snTerm->setEnabled( false ) ;
		m_function( signal::term ) ;
#else
		snTerm->setEnabled( false ) ;

		char tmp ;

		::read( sighupFd[ 1 ],&tmp,sizeof( tmp ) ) ;

		m_function( signal::term ) ;

		snTerm->setEnabled( true ) ;
#endif
	} ) ;
}

#endif

#ifdef Q_OS_WIN

#include <QApplication>
#include <QAbstractNativeEventFilter>

#include "utility.h"

#include <windows.h>

class eventFilter : public QObject,public QAbstractNativeEventFilter
{
public:
	eventFilter( QObject * parent,std::function< void( systemSignalHandler::signal ) > function ) :
		QObject( parent ),m_function( std::move( function ) )
	{
	}
	bool nativeEventFilter( const QByteArray& eventType,void * message,long * result )
	{
		Q_UNUSED( eventType ) ;
		Q_UNUSED( result ) ;

		MSG * msg = reinterpret_cast< MSG * >( message ) ;

		if( msg->message == WM_ENDSESSION ){

			m_function( systemSignalHandler::signal::winEndSession ) ;

			//return true ;
		}

		return false ;
	}

	std::function< void( systemSignalHandler::signal ) > m_function ;
};

systemSignalHandler::systemSignalHandler( QObject * parent,std::function< void( signal ) > function ) :
	m_parent( parent ),m_function( std::move( function ) )
{
}

void systemSignalHandler::listen()
{
	if( settings::instance().unMountVolumesOnLogout() ){

		auto m = new eventFilter( m_parent,std::move( m_function ) ) ;
		QApplication::instance()->installNativeEventFilter( m ) ;
	}
}

#endif

#ifdef Q_OS_MACOS

systemSignalHandler::systemSignalHandler( QObject * parent,std::function< void( signal ) > function )
	: m_parent( parent ),m_function( std::move( function ) )
{
}

void systemSignalHandler::listen()
{
}

#endif

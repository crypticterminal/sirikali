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

#include "sshfs.h"
#include "commandOptions.h"

sshfs::sshfs() : backEnd::engine( "sshfs" )
{
}

const QStringList& sshfs::names() const
{
	return m_names ;
}

const QStringList& sshfs::fuseNames() const
{
	return m_fuseNames ;
}

siritask::status sshfs::notFoundCode() const
{
	return siritask::status::sshfsNotFound ;
}

QString sshfs::command( const backEnd::cmdArgsList& args ) const
{
	commandOptions m( args,m_names.first(),m_names.first() ) ;

	auto fuseOptions = m.fuseOpts() ;

	if( !args.opt.key.isEmpty() ){

		fuseOptions.add( "password_stdin" ) ;
	}

	if( fuseOptions.doesNotContain( "StrictHostKeyChecking=" ) ){

		fuseOptions.addPair( "StrictHostKeyChecking","no" ) ;
	}

	auto exeOptions = m.exeOptions() ;

	if( utility::platformIsWindows() ){

		exeOptions.add( "-f" ) ;
	}

	QString s = "%1 %2 %3 %4 %5" ;

	return s.arg( args.exe,
		      exeOptions.get(),
		      args.cipherFolder,
		      args.mountPoint,
		      fuseOptions.get() ) ;
}

siritask::status sshfs::errorCode( const QString& e,int s ) const
{
	Q_UNUSED( s ) ;

	if( e.contains( "password" ) ){

		return siritask::status::sshfsBadPassword ;

	}else if( e.contains( "winfsp" ) ){

		return siritask::status::failedToLoadWinfsp ;
	}else{
		return siritask::status::backendFail ;
	}
}

bool sshfs::setsCipherPath() const
{
	return true ;
}

QString sshfs::configFileArgument() const
{
	return QString() ;
}

QStringList sshfs::configFileNames() const
{
	return {} ;
}

bool sshfs::autoMountsOnCreate() const
{
	return true ;
}

QString sshfs::setPassword( const QString& e ) const
{
	auto m = e ;

	/*
	 * On my linux box, sshfs prompts six times when entered password is wrong before
	 * giving up, here, we simulate replaying the password 10 times hoping it will be
	 * enough for sshfs.
	 */
	for( int i = 0 ; i < 9 ; i++ ){

		m += "\n" + m ;
	}

	return m ;
}

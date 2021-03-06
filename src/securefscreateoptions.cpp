/*
 *
 *  Copyright (c) 2017
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

#include "securefscreateoptions.h"
#include "ui_securefscreateoptions.h"

#include "utility.h"

securefscreateoptions::securefscreateoptions( QWidget * parent,
					      std::function< void( const Options& ) > function ) :
	QDialog( parent ),
	m_ui( new Ui::securefscreateoptions ),
	m_function( std::move( function ) )
{
	m_ui->setupUi( this ) ;

	this->setFixedSize( this->window()->size() ) ;

	connect( m_ui->pbOK,SIGNAL( clicked() ),this,SLOT( pbOK() ) ) ;
	connect( m_ui->pbCancel,SIGNAL( clicked() ),this,SLOT( pbCancel() ) ) ;
	connect( m_ui->pbConfigFile,SIGNAL( clicked() ),this,SLOT( pbConfigFilePath() ) ) ;

	m_ui->pbConfigFile->setIcon( QIcon( ":/folder.png" ) ) ;

	m_ui->comboBox->setFocus() ;

	this->show() ;
}

securefscreateoptions::~securefscreateoptions()
{
	delete m_ui ;
}

void securefscreateoptions::pbOK()
{
	this->hide() ;

	auto e = m_ui->lineEdit->text() ;

	if( !e.isEmpty() ){

		e = "--config " + e ;
	}

	if( m_ui->comboBox->currentIndex() == 1 ){

		this->HideUI( { { "--format 2",m_ui->lineEdit->text() } } ) ;
	}else{
		this->HideUI( { { "--format 4",m_ui->lineEdit->text() } } ) ;
	}
}

void securefscreateoptions::pbCancel()
{
	this->HideUI() ;
}

void securefscreateoptions::pbConfigFilePath()
{
	m_ui->lineEdit->setText( utility::configFilePath( this,"securefs" ) ) ;
}

void securefscreateoptions::HideUI( const Options& opts )
{
	this->hide() ;
	m_function( opts ) ;
	this->deleteLater() ;
}

void securefscreateoptions::closeEvent( QCloseEvent * e )
{
	e->ignore() ;
	this->pbCancel() ;
}

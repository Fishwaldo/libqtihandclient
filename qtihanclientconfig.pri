QIC_VER_MAJ      = 0
QIC_VER_MIN      = 1
isEmpty(QIC_VER_PAT) {
	QIC_VER_PAT      = 0
}
VERSION      = $${QIC_VER_MAJ}.$${QIC_VER_MIN}.$${QIC_VER_PAT}



unix { 
	isEmpty(PREFIX) {
		PREFIX=/usr/local
	}


	isEmpty(DOCDIR) {
		QIC_INSTALL_DOCS     = $${PREFIX}/share/doc/Qt$${QT_MAJOR_VERSION}iHanClient-$${VERSION}/
	} else {
		QIC_INSTALL_DOCS	  = $${DOCDIR}/Qt$${QT_MAJOR_VERSION}iHanClient-$${VERSION}
	}	 
	isEmpty(INCDIR) {
		QIC_INSTALL_HEADERS   = $${PREFIX}/include/qt$${QT_MAJOR_VERSION}/QtiHanClient
		QIC_HEADERS_PATH      = $${PREFIX}/include/qt$${QT_MAJOR_VERSION}
	} else {
		QIC_INSTALL_HEADERS	= $${INCDIR}/qt$${QT_MAJOR_VERSION}/QtiHanClient
		QIC_HEADERS_PATH      	= $${INCDIR}/qt$${QT_MAJOR_VERSION}
	}
	isEmpty(LIBDIR) {
		QIC_INSTALL_LIBS      = $${PREFIX}/lib64/
	} else {
		QIC_INSTALL_LIBS	  = $${LIBDIR}
	}
	isEmpty(PKGCONFIGDIR) {
		TMP_PATH 			  = $$system(pkg-config pkg-config --variable=pc_path)
		QIC_INSTALL_PC		  = $$section(TMP_PATH, :, 0, 0)
	} else {
		QIC_INSTALL_PC		  = $${PKGCONFIGDIR}
	}
	message(Docs Install Path: $${QIC_INSTALL_DOCS})
	message(Include Install Path: $${QIC_INSTALL_HEADERS})
	message(Lib Install Path: $${QIC_INSTALL_LIBS})
	message(PkgConfig Path: $${QIC_INSTALL_PC})
}
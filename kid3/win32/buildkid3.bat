%MSYSDIR%\bin\perl ../kid3-qt/configure-kid3-qt.pl --generate-ts --prefix= --with-bindir= --with-datarootdir= --with-docdir= --with-translationsdir= --without-musicbrainz --enable-gcc-pch --with-db2html="%XSLTPROCDIR%\xsltproc --novalid --nonet" --with-xsl-stylesheet=%DOCBOOKDIR%\html\docbook.xsl --with-perl-cmd=%MSYSDIR%\bin\perl --with-extra-includes=%MSYSDIR%\local\include --with-taglib-includes=-I%MSYSDIR%\local\include\taglib --with-extra-defines="ID3LIB_LINKOPTION=1 FLAC__NO_DLL" --with-extra-libs=-L%MSYSDIR%\local\lib
mkdir kid3win
make && make install INSTALL_ROOT=%cd%\kid3win

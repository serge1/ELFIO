xsltproc -o elfio.fo /usr/share/xml/docbook/stylesheet/docbook-xsl/fo/docbook.xsl elfio.docbook
fop -fo elfio.fo -pdf elfio.pdf

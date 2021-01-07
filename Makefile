build: .build

upload: build
	pio run -t upload

.build: src/html.h $(wildcard src/*)
	pio run
	touch $@

src/html.h: data/index.html
	echo "#define INDEX_HTML F(\"$$(minify $< | sed 's#"#\\"#g')\")" > $@

.PHONY: build upload

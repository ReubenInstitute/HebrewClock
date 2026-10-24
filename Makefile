PYPI_TOKEN ?= ~/pypi.txt

.PHONY: pip install-pip upload clean

pip:
	rm -rf build
	mkdir -p build/hebrewclock
	cp yemama.py clock.py display.py custom.py __init__.py build/
	cp pip/pyproject.toml build/
	cp LICENSE build/
	cp README.md build/
	( cd build && python3 -m build --outdir . )

install-pip:
	python3 -m pip install build/*.whl

upload:
	TWINE_USERNAME=__token__ TWINE_PASSWORD=$$(cat $(PYPI_TOKEN)) twine upload build/*.whl build/*.tar.gz

clean:
	rm -rf build
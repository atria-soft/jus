Build lib & build sample                           {#zeus_build}
========================

@tableofcontents

Download:                                          {#zeus_build_download}
=========

ege use some tools to manage source and build it:

need google repo:                                  {#zeus_build_download_repo}
-----------------

see: http://source.android.com/source/downloading.html#installing-repo

On all platform:
```{.sh}
	mkdir ~/.bin
	PATH=~/.bin:$PATH
	curl https://storage.googleapis.com/git-repo-downloads/repo > ~/.bin/repo
	chmod a+x ~/.bin/repo
```

On ubuntu
```{.sh}
	sudo apt-get install repo
```

On archlinux
```{.sh}
	sudo pacman -S repo
```

lutin (build-system):                              {#zeus_build_download_lutin}
---------------------

```{.sh}
	pip install lutin --user
	# optionnal dependency of lutin (manage image changing size for application release)
	pip install pillow --user
```


dependency:                                        {#zeus_build_download_dependency}
-----------

```{.sh}
	mkdir -p WORKING_DIRECTORY/framework
	cd WORKING_DIRECTORY/framework
	repo init -u git://github.com/atria-soft/manifest.git
	repo sync -j8
	cd ../..
```

sources:                                           {#zeus_build_download_sources}
--------

They are already download in the repo manifest in:

```{.sh}
	cd WORKING_DIRECTORY/framework/atria-soft/zeus
```

Build:                                             {#zeus_build_build}
======

you must stay in zour working directory...
```{.sh}
	cd WORKING_DIRECTORY
```

library:                                           {#zeus_build_build_library}
--------

```{.sh}
	lutin -mdebug zeus
```

Sample:                                            {#zeus_build_build_sample}
-------

```{.sh}
	# generic gateway (with acces control)
	lutin -mdebug zeus-gateway
	# control acces with the user property
	lutin -mdebug zeus-system-user
	# simple picture service (example)
	lutin -mdebug zeus-picture
	# test service
	lutin -mdebug zeus-test-service1
	# test client
	lutin -mdebug zeus-test-client
```

A fast way:
```{.sh}
	lutin -mdebug zeus-*
```


Run sample:                                        {#zeus_build_run_sample}
===========

you must copy a configuration file:

```{.sh}
	mkdir -p ~/.local/share/zeus-system-user/
	cp framework/atria-soft/zeus/test1#atria-soft.com.json  ~/.local/share/zeus-system-user/test1~atria-soft.com.json
```

in distinct bash:
```{.sh}
	lutin -mdebug zeus-gateway?run
	lutin -mdebug zeus-system-user?run
	lutin -mdebug zeus-picture?run
	lutin -mdebug zeus-test-service1?run
	lutin -mdebug zeus-test-client?run
```

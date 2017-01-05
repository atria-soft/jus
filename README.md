zeus
====

`zeus` is a Zeus ewol µ-service

Instructions
============

messaging engine drived by data and message based on websocket api

Start basic service engine
==========================

Start The router interface:
```
lutin -cclang -mdebug zeus-package-base?build?run%zeus-router
```

You have now multiple choice:

* Single process start:

```
#Start a single gateWay with basic with no user service associated:
lutin -cclang -mdebug zeus-package-base?build?run%zeus-gateway:--user=userName~server.org
# start service is separated process: (the user service is needed all the time ...)
lutin -cclang -mdebug zeus-package-base?build?run%zeus-launch:--srv=user
lutin -cclang -mdebug zeus-package-base?build?run%zeus-launch:--srv=picture
lutin -cclang -mdebug zeus-package-base?build?run%zeus-launch:--srv=video
```

* Start your gateway with the service in a single process (faster: No inter-process messaging)

```
lutin -cclang -mdebug zeus-package-base?build?run%zeus-gateway:--user=userName~server.org:--srv=user:--srv=picture:--srv=video
```


License (MPL v2.0)
=====================
Copyright zeus Edouard DUPIN

Licensed under the Mozilla Public License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.mozilla.org/MPL/2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


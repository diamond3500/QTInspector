#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements. See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership. The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License. You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
# KIND, either express or implied. See the License for the
# specific language governing permissions and limitations
# under the License.
#
find_package(QT NAMES Qt6 Qt5 REQUIRED Core)
find_package(Felgo QUIET)
if (Felgo_FOUND)
    add_subdirectory(felgo)
    set_target_properties(appfelgo PROPERTIES FOLDER "example/appfelgo")
endif()


if(${QT_VERSION} VERSION_LESS 6.2.0)
    message("appqml version less than 6.2, current version:" ${QT_VERSION})
else()
    add_subdirectory(appqml)
    set_target_properties(appqml PROPERTIES FOLDER "example/appqml")
endif()

if(${QT_VERSION} VERSION_LESS 5.12.0)
    message("appwidget version less than 5.12, current version:" ${QT_VERSION})
else()
    add_subdirectory(appwidget)
    set_target_properties(appwidget PROPERTIES FOLDER "example/appwidget")
endif()

# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [v1.0.4] - 2021-05-20

### Added

-   Add game matrix access [#161](https://github.com/XLabsProject/s1x-client/issues/161)
-   Add custom camo support [#185](https://github.com/XLabsProject/s1x-client/issues/185)

### Fixed

-   Fatal error with test build [#193](https://github.com/XLabsProject/s1x-client/issues/193)

### Pull Requests

-   Fix service not available disconnect [#172](https://github.com/XLabsProject/s1x-client/pull/172) ([@Joelrau](https://github.com/Joelrau))
-   Script struct support #132 [#177](https://github.com/XLabsProject/s1x-client/pull/177) ([@fedddddd](https://github.com/fedddddd))
-   Use sizeof operator more often [#181](https://github.com/XLabsProject/s1x-client/pull/181) ([@diamante0018](https://github.com/diamante0018))
-   Script function support [#183](https://github.com/XLabsProject/s1x-client/pull/183) ([@fedddddd](https://github.com/fedddddd))

## [v1.0.3] - 2021-05-04

### Added

-   Use patrons for bot names [#128](https://github.com/XLabsProject/s1x-client/issues/128)
-   Increase network packet limit [#102](https://github.com/XLabsProject/s1x-client/issues/102)

### Fixed

-   Possible duplicate GUID [#166](https://github.com/XLabsProject/s1x-client/issues/166)

### Pull Requests

-   Game variable support #161 [#164](https://github.com/XLabsProject/s1x-client/pull/164) ([@fedddddd](https://github.com/fedddddd))

## [v1.0.2] - 2021-04-29

### Added

-   Add kick reason [#114](https://github.com/XLabsProject/s1x-client/issues/114)
-   Improve MOTD with social links and whatever [#131](https://github.com/XLabsProject/s1x-client/issues/131)
-   Implement Listassetpool filter similar to IW6x [#123](https://github.com/XLabsProject/s1x-client/issues/123)

### Fixed

-   Unable to connect to servers [#106](https://github.com/XLabsProject/s1x-client/issues/106)

### Pull Requests

-   Use dvar flag for r_fullbright [#142](https://github.com/XLabsProject/s1x-client/pull/142) ([@diamante0018](https://github.com/diamante0018))
-   List Asset Pool Filter [#134](https://github.com/XLabsProject/s1x-client/pull/134) ([@diamante0018](https://github.com/diamante0018))
-   Fixed logic error in list Asset pool command [#136](https://github.com/XLabsProject/s1x-client/pull/136) ([@diamante0018](https://github.com/diamante0018))
-   Show drop reason [#147](https://github.com/XLabsProject/s1x-client/pull/147) ([@Joelrau](https://github.com/Joelrau))
-   Small QOL changes mostly for dedicated server owners. [#125](https://github.com/XLabsProject/s1x-client/pull/125) ([@xerxes-at](https://github.com/xerxes-at))

## [v1.0.1] - 2021-04-26

### Added

-   Move the chat [#99](https://github.com/XLabsProject/s1x-client/issues/99)
-   Reduce AntiVirus false positives [#127](https://github.com/XLabsProject/s1x-client/issues/127)
-   Increase timeout time (when connecting?) [#139](https://github.com/XLabsProject/s1x-client/issues/139)

### Fixed

-   Campaign fails to launch in new debug builds [#116](https://github.com/XLabsProject/s1x-client/issues/116)
-   VirtualLobby firing range FOV/FOV scale messed up [#135](https://github.com/XLabsProject/s1x-client/issues/135)

### Pull Requests

-   Update scripting [#95](https://github.com/XLabsProject/s1x-client/pull/95) ([@Joelrau](https://github.com/Joelrau))
-   Fixed hooks in set dvars [#100](https://github.com/XLabsProject/s1x-client/pull/100) ([@diamante0018](https://github.com/diamante0018))
-   hooked dvar register vector2, modified cg_hudChatPosition and matched... [#103](https://github.com/XLabsProject/s1x-client/pull/103) ([@diamante0018](https://github.com/diamante0018))
-   I missed flags in vector 2 yesterday. [#110](https://github.com/XLabsProject/s1x-client/pull/110) ([@diamante0018](https://github.com/diamante0018))
-   use dvar flags enum everywhere for extra clarity and cohesion. also s... [#115](https://github.com/XLabsProject/s1x-client/pull/115) ([@diamante0018](https://github.com/diamante0018))
-   Update patches.cpp [#119](https://github.com/XLabsProject/s1x-client/pull/119) ([@diamante0018](https://github.com/diamante0018))
-   /reconnect changes [#122](https://github.com/XLabsProject/s1x-client/pull/122) ([@mjkzy](https://github.com/mjkzy))
-   Use custom fov/fovscale when virtual lobby is enabled. [#133](https://github.com/XLabsProject/s1x-client/pull/133) ([@Joelrau](https://github.com/Joelrau))
-   filter. (tested on mp) [#134](https://github.com/XLabsProject/s1x-client/pull/134) ([@diamante0018](https://github.com/diamante0018))
-   reverted to old if statement [#136](https://github.com/XLabsProject/s1x-client/pull/136) ([@diamante0018](https://github.com/diamante0018))
-   Fix firing range fov [#137](https://github.com/XLabsProject/s1x-client/pull/137) ([@Joelrau](https://github.com/Joelrau))
-   Don't patch virtuallobby if game isn't multiplayer [#138](https://github.com/XLabsProject/s1x-client/pull/138) ([@Joelrau](https://github.com/Joelrau))

## [v1.0.0] - 2021-04-20

### Added

-   Uncheat protect safeArea & give SAVED flags [#75](https://github.com/XLabsProject/s1x-client/issues/75)
-   Create unified logging interface [#78](https://github.com/XLabsProject/s1x-client/issues/78)

### Fixed

-   Input is lagging in fullscreen mode [#69](https://github.com/XLabsProject/s1x-client/issues/69)
-   GUID changing upon game restart [#74](https://github.com/XLabsProject/s1x-client/issues/74)
-   Duplicated GUIDs [#90](https://github.com/XLabsProject/s1x-client/issues/90)
-   Serverlist connect issue [#92](https://github.com/XLabsProject/s1x-client/issues/92)

### Pull Requests

-   discord presence final commit [#66](https://github.com/XLabsProject/s1x-client/pull/66) ([@mjkzy](https://github.com/mjkzy))
-   Feature/scripting [#70](https://github.com/XLabsProject/s1x-client/pull/70) ([@Joelrau](https://github.com/Joelrau))
-   Feature/quaK [#81](https://github.com/XLabsProject/s1x-client/pull/81) ([@Joelrau](https://github.com/Joelrau))
-   unlock safeArea [#82](https://github.com/XLabsProject/s1x-client/pull/82) ([@mjkzy](https://github.com/mjkzy))
-   hot fix safe area [#83](https://github.com/XLabsProject/s1x-client/pull/83) ([@mjkzy](https://github.com/mjkzy))

## [v0.0.2] - 2021-04-07

### Added

-   Offline mode [#19](https://github.com/XLabsProject/s1x-client/issues/19)
-   Rewrite demonware emulator [#20](https://github.com/XLabsProject/s1x-client/issues/20)
-   Add a way to disable auto restart on crashes. [#58](https://github.com/XLabsProject/s1x-client/issues/58)

### Fixed

-   Investigate performance issues [#11](https://github.com/XLabsProject/s1x-client/issues/11)
-   Map command ingame [#13](https://github.com/XLabsProject/s1x-client/issues/13)
-   Replicated dvars are reset upon map start/restart [#14](https://github.com/XLabsProject/s1x-client/issues/14)
-   Process sometimes doesn't exit [#17](https://github.com/XLabsProject/s1x-client/issues/17)
-   Steam Files causing Outdated Game Files error. [#39](https://github.com/XLabsProject/s1x-client/issues/39)
-   Killcam Lags/Skips [#53](https://github.com/XLabsProject/s1x-client/issues/53)

### Pull Requests

-   More cleanup [#1](https://github.com/XLabsProject/s1x-client/pull/1) ([@OneFourOne](https://github.com/OneFourOne))
-   Quick README change [#2](https://github.com/XLabsProject/s1x-client/pull/2) ([@ChxseH](https://github.com/ChxseH))
-   fix typo [#3](https://github.com/XLabsProject/s1x-client/pull/3) ([@OneFourOne](https://github.com/OneFourOne))
-   Fix Joelrau/s1-mod#9 [#5](https://github.com/XLabsProject/s1x-client/pull/5) ([@fedddddd](https://github.com/fedddddd))
-   Unlock everything and re-enable virtuallobby [#24](https://github.com/XLabsProject/s1x-client/pull/24) ([@Joelrau](https://github.com/Joelrau))
-   Add listassetpool, consoleList, fix various problems [#32](https://github.com/XLabsProject/s1x-client/pull/32) ([@iAmThatMichael](https://github.com/iAmThatMichael))
-   Allow kbam while gamepad is enabled & Logger stuff [#33](https://github.com/XLabsProject/s1x-client/pull/33) ([@Joelrau](https://github.com/Joelrau))
-   add discord presence [#36](https://github.com/XLabsProject/s1x-client/pull/36) ([@mjkzy](https://github.com/mjkzy))
-   Minor fixes [#40](https://github.com/XLabsProject/s1x-client/pull/40) ([@Joelrau](https://github.com/Joelrau))
-   Uploading new artworks + resources [#41](https://github.com/XLabsProject/s1x-client/pull/41) ([@sortileges](https://github.com/sortileges))
-   Make it possible to open the client without console [#45](https://github.com/XLabsProject/s1x-client/pull/45) ([@Joelrau](https://github.com/Joelrau))
-   Fix slowmotion on dedicated [#54](https://github.com/XLabsProject/s1x-client/pull/54) ([@Joelrau](https://github.com/Joelrau))
-   Discord RPC - party size + party size max [#59](https://github.com/XLabsProject/s1x-client/pull/59) ([@mjkzy](https://github.com/mjkzy))
-   discord presence - host name address [#64](https://github.com/XLabsProject/s1x-client/pull/64) ([@mjkzy](https://github.com/mjkzy))

[Unreleased]: https://github.com/XLabsProject/s1x-client/compare/v1.0.4...HEAD

[v1.0.4]: https://github.com/XLabsProject/s1x-client/compare/v1.0.3...v1.0.4

[v1.0.3]: https://github.com/XLabsProject/s1x-client/compare/v1.0.2...v1.0.3

[v1.0.2]: https://github.com/XLabsProject/s1x-client/compare/v1.0.1...v1.0.2

[v1.0.1]: https://github.com/XLabsProject/s1x-client/compare/v1.0.0...v1.0.1

[v1.0.0]: https://github.com/XLabsProject/s1x-client/compare/v0.0.2...v1.0.0

[v0.0.2]: https://github.com/XLabsProject/s1x-client/compare/75b6d04895a2da346ca9eba5352b300f4926b6c5...v0.0.2

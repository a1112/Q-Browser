# QML Demo local validation — 2026-09-15

Environment: Windows 11, Qt 6.11.0 MSVC 2022 x64, Release Host/Worker.
Deployment: `C:\QBrowserQmlDemos\release-deploy` (development, no production attestation).

## Automated checks

| Check | Result | Local evidence |
| --- | --- | --- |
| QML loading, legal page metadata, serialized audio requests, coffee workflow, local social state | 8 passed | `build/qml-demo-final-interactions.txt` |
| Runtime coordinator including independent package selection, missing package and altered package rejection | 37 passed | `build/qml-coordinator-tests.txt` |
| Real Qt Multimedia playback, position, duration, seeking, pause, revoked authority and invalid track requests | 3 passed, no skips | `build/audio-broker-test2.txt` |
| Browser shell, gallery, performance monitor and browser chrome | All 4 suites passed | `build/qml-regressions.txt` |
| Manifest, effective policy and Host capability runtime | Passed | `build/qml-core-tests.txt` (the earlier gallery failure in this log is superseded by the regression log) |
| Bundled QML static source policy | Passed | `build/qml-source-test2.txt` |
| Signed development deployment, PE dependencies, hashes and protected paths | Passed | `build/qml-deployment-validated.log` |

## Actual deployed windows

Installed Pilot and all three independently signed Demo packages through the
production Host. Opened the three routes using the Pilot-configured Host and
confirmed the corresponding QML UI and titles. Opened Tokodon and Elisa in
separate tabs; Tokodon detail state survived switching between them.

Elisa's WAV playback reached a real backend position of 19 seconds after a
background tab switch. No busy-request error appeared after serialization.
Next-track playback selected the second WAV. Closing the playing Elisa tab
reduced the Worker count from two to one. Closing the Host reclaimed the
remaining Worker. An offline restart restored the Tokodon route from its own
installed package, with session-only social data reset.

Coffee's actual window also completed selection of cappuccino, recipe
confirmation, animated preparation, completion and explicit restart to Home.

Screenshots are captured from actual deployed windows, not rendered mockups:
`build/qml-demo-screenshots/{gallery,elisa,tokodon,coffee,coffee-recipe,coffee-home,coffee-complete}.png`.
The audio test confirms the real backend and output device; it does not measure
speaker acoustic output. Device hot-unplug and every physical DPI/monitor
combination are not exhaustively covered by this local run.

<!--
SPDX-FileCopyrightText: 2021-2026 Roy Shilkrot <roy.shil@gmail.com>
SPDX-FileCopyrightText: 2023-2026 Kaito Udagawa <umireon@kaito.tokyo>
SPDX-FileCopyrightText: 2026 Manoel Gerlach <mail@manoel.us>

SPDX-License-Identifier: GPL-3.0-or-later
-->

# OBS Background Removal — Windows DirectML Fork

> [!IMPORTANT]
> **Unofficial modified fork**
>
> This repository is an unofficial modified fork of the open-source
> [obs-backgroundremoval](https://github.com/royshil/obs-backgroundremoval) project. It is not an official
> upstream release.
>
> - **Original authors:** [Roy Shilkrot (royshil)](https://github.com/royshil) and
>   [Kaito Udagawa (umireon)](https://github.com/umireon)
> - **Fork maintainer:** [Manoel Gerlach](https://github.com/manoelgerlach)
> - **Current fork version:** `1.4.1-directml.1`
> - **Primary fork feature:** Windows x64 ONNX Runtime DirectML GPU inference

This fork preserves the original project's portrait background removal, virtual green-screen, and low-light
enhancement features while restoring Windows GPU inference through the ONNX Runtime DirectML Execution Provider.

<div align="center">

[![License](https://img.shields.io/github/license/manoelgerlach/obs-backgroundremoval)](LICENSE)
[![Windows PR Check](https://github.com/manoelgerlach/obs-backgroundremoval/actions/workflows/pr-check.yml/badge.svg?branch=feature/windows-directml)](https://github.com/manoelgerlach/obs-backgroundremoval/actions/workflows/pr-check.yml)

</div>

A plugin for [OBS Studio](https://obsproject.com/) that allows you to replace the background in portrait images and video, as well as enhance low-light scenes.

## Fork downloads and Windows installation

### Fork Releases

No formal fork release has been published yet. Development packages are produced by the repository's
[GitHub Actions workflows](https://github.com/manoelgerlach/obs-backgroundremoval/actions). A formal release will
follow after release-readiness work is complete.

The validated Windows x64 development artifact is named:

`obs-backgroundremoval_1.4.1-directml.1-windows-x64.zip`

### Install the Windows x64 package

1. Close OBS Studio.
2. Extract the ZIP without changing its internal directory structure.
3. Copy the extracted `obs-backgroundremoval` directory to:

   `C:\ProgramData\obs-studio\plugins\obs-backgroundremoval`

4. Confirm that the installed layout includes at least:

   ```text
   C:\ProgramData\obs-studio\plugins\obs-backgroundremoval\
   ├── bin\64bit\obs-backgroundremoval.dll
   ├── bin\64bit\DirectML.dll
   └── data\
       ├── effects\
       ├── locale\
       └── models\mediapipe.onnx
   ```

5. Start OBS Studio and add the **Background Removal** filter. Select **GPU - DirectML** as the inference device
   when Windows GPU inference is desired.

`DirectML.dll` is included beside the plugin DLL in `bin\64bit`; no separate DirectML runtime download is required.

### Original / Upstream Releases

Official upstream releases remain available from the
[original project's releases page](https://github.com/royshil/obs-backgroundremoval/releases). Those releases are
published by the upstream project and are not releases or updates of this DirectML fork.

For an alternative maintained by original author Kaito Udagawa, see
[Live Background Removal Lite](https://github.com/kaito-tokyo/live-backgroundremoval-lite).

## Usage

<div style="text-align:center;">
<video src="https://github.com/royshil/obs-backgroundremoval/assets/1067855/5ba5aae2-7ea2-4c90-ad45-fba5ccde1a4e" width="320"></video>
</div>

The [original project's usage guide](https://royshil.github.io/obs-backgroundremoval/usage/) provides usage
walkthroughs and general recommendations. It documents upstream behavior and may not cover this fork's DirectML
changes.

Additional tutorial videos:

- [▶︎ Official guide to the Background Removal plugin for OBS Studio on YouTube](https://www.youtube.com/playlist?list=PLfd4SnaQQz_DVr_18OQozucYmiC56rRhy)
- Depth of Field effect: https://youtu.be/jC3EKSpNjQk
- Low-light enhancement: https://youtu.be/WSBLYWFrn2Q
- Remove background from ANY object (not just human): https://youtu.be/N74VCDCToX8

## Building from source

### Windows DirectML fork

```pwsh
git clone --recurse-submodules https://github.com/manoelgerlach/obs-backgroundremoval.git
cd obs-backgroundremoval
git switch feature/windows-directml
```

The canonical validated Windows build path is the repository's
[`PR Check` workflow](.github/workflows/pr-check.yml), which invokes
[`Build (Windows)`](.github/workflows/build-windows.yml). It builds static ONNX Runtime 1.28 with DirectML enabled,
packages the matching `DirectML.dll`, runs the DirectML MediaPipe smoke test, and uploads the Windows x64 ZIP.

The local Windows helper scripts are not the canonical build path for this fork.

### Original cross-platform build information

The original project supports building Debian Forky packages with:

```sh
git clone https://github.com/royshil/obs-backgroundremoval.git
cd obs-backgroundremoval
sudo ./bin/bootstrap
./bin/setup
./bin/build
sudo dpkg -i release/obs-backgroundremoval-*-linux-gnu.deb
```

That section describes upstream/cross-platform development, not the validated Windows DirectML fork build.

## Introduction

This plugin is meant to make it easy to replace the background in portrait images and video.
It is using a neural network to predict the mask of the portrait and remove the background pixels.
It's easily composable with other OBS plugins to replace the background with e.g. an image or
a transparent color.

To support the original work on which this fork is based, consider sponsoring the upstream authors:

- https://github.com/sponsors/royshil
- https://github.com/sponsors/umireon

### Support and Help

Report fork-specific bugs, including Windows or DirectML problems, through the
[fork issue tracker](https://github.com/manoelgerlach/obs-backgroundremoval/issues).

The [upstream GitHub Discussions](https://github.com/royshil/obs-backgroundremoval/discussions),
[upstream issue tracker](https://github.com/royshil/obs-backgroundremoval/issues), and
[OBS Plugins forum](https://obsproject.com/forum/resources/background-removal-portrait-segmentation.1260/) remain
useful resources for the original project. Roy Shilkrot and Kaito Udagawa are not responsible for supporting this
unofficial fork.

For upstream hands-on help or private consultation, see
[Roy Shilkrot's sponsorship options](https://github.com/sponsors/royshil?frequency=one-time).

### Technical Details

GPU support:

- This fork's implemented Windows GPU backend is the **ONNX Runtime DirectML Execution Provider**, not Windows ML.
- The validated fork configuration supports Windows 11 x64, ONNX Runtime 1.28, MediaPipe segmentation through
  DirectML, explicit CPU inference selection, and the packaged DirectML runtime.
- The Windows package includes `DirectML.dll` beside `obs-backgroundremoval.dll`.
- DirectML availability and performance depend on the Windows, GPU, and driver configuration. No specific speedup is
  guaranteed.
- CPU inference remains available as an explicit selection. A DirectML initialization failure is reported and is not
  silently presented as successful GPU inference.

Upstream/cross-platform notes, not revalidated as part of this Windows fork work:

- On macOS, the original project supports CoreML acceleration on Apple Silicon. The plugin does not support
  cross-architecture translation through Rosetta 2.
- Linux GPU execution-provider availability depends on how ONNX Runtime and the plugin are built.

The number of CPU inference threads remains configurable through the plugin UI.

The pretrained model weights used for portrait foreground segmentation are taken from:

- https://github.com/anilsathyan7/Portrait-Segmentation/tree/master/SINet
- https://github.com/PaddlePaddle/PaddleSeg/tree/release/2.7/contrib/PP-HumanSeg
- https://github.com/PINTO0309/PINTO_model_zoo/tree/main/082_MediaPipe_Meet_Segmentation
- https://github.com/PeterL1n/RobustVideoMatting
- https://github.com/PINTO0309/PINTO_model_zoo/tree/main/384_TCMonoDepth and https://github.com/yu-li/TCMonoDepth

Image enhancement (low light) models are taken from:

- https://github.com/PINTO0309/PINTO_model_zoo/tree/main/213_TBEFN
- https://github.com/PINTO0309/PINTO_model_zoo/tree/main/372_URetinex-Net
- https://github.com/PINTO0309/PINTO_model_zoo/tree/main/370_Semantic-Guided-Low-Light-Image-Enhancement

Some more information about how I built it: https://www.morethantechnical.com/2021/04/15/obs-plugin-for-portrait-background-removal-with-onnx-sinet-model/ and https://www.morethantechnical.com/2023/05/20/building-an-obs-background-removal-plugin-a-walkthrough/

### Code Walkthrough

This video on YouTube will take you through the major parts of the code and explain them.

<div align="center">
  <a href="https://youtu.be/iFQtcJg0Wsk" target="_blank">
    <img width="50%" src="https://img.youtube.com/vi/iFQtcJg0Wsk/maxresdefault.jpg"/>
  </a>
</div>

## Download Trends and Star History

<div align="center">

![Download Trends](docs/download-trends.svg)
![GitHub Star History](docs/star-history.svg)

</div>

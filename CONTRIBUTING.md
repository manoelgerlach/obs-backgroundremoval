<!--
SPDX-FileCopyrightText: 2026 Kaito Udagawa <umireon@kaito.tokyo>
SPDX-FileCopyrightText: 2026 Manoel Gerlach <mail@manoel.us>

SPDX-License-Identifier: Apache-2.0
-->

# Contributing to the Windows DirectML fork

This repository, [`manoelgerlach/obs-backgroundremoval`](https://github.com/manoelgerlach/obs-backgroundremoval), is
an unofficial Windows DirectML fork of the original
[`royshil/obs-backgroundremoval`](https://github.com/royshil/obs-backgroundremoval) project by Roy Shilkrot
(`royshil`) and Kaito Udagawa (`umireon`). Fork-specific issues and pull requests must be filed against this fork,
not against the original maintainers.

The primary supported development target is Windows 11 x64 with OBS Studio, static ONNX Runtime, and DirectML GPU
inference. Relevant Windows x64 CI must pass. Changes must not break DirectML behavior without explicit technical
justification and review.

## Legal and Community Guidelines

To keep this fork safe and reliable for users and contributors, everyone involved in its development must follow
these terms. Pull requests may be asked to confirm compliance with this policy.

### 1. Licensing and Patent Grant

- **GPL-3.0-or-later**: You agree to distribute your contribution of C and C++ code under the GNU General Public License v3.0 or later.
- **Patent License**: Refer to the [terms of GNU GPLv3 (Section 11)](https://www.gnu.org/licenses/gpl-3.0.html#section11) about patents.

### 2. Code Integrity

We always welcome every contribution! To deliver the value of your work to our users in a professional manner, please adhere to the following policies:

- **AI-assisted development**: AI-assisted development is permitted, but contributors remain fully responsible for
  understanding, reviewing, testing, licensing, and explaining all submitted code. Unreviewed, unexplained, or
  blindly submitted generated code is not acceptable.
- **Responsibility**: You must take responsibility for your code and ensure that your contribution is reliable and
  has clean licensing.

### 3. Developer Certificate of Origin (DCO) and Commit Signing

Apply DCO sign-off (`-s`) to every commit and cryptographically sign every commit (`-S`) with a GPG or SSH key:

```
git commit -s -S
```

Please refer to the [GitHub Document for commit signing](https://docs.github.com/authentication/managing-commit-signature-verification) for configuring commit signing.

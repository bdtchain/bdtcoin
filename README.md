Bdtcoin Core integration/staging tree
=====================================

https://bdtcoin.org

What is Bdtcoin?
----------------

BDTCOIN is an innovative decentralized cryptocurrency designed to facilitate direct exchanges of value between users within a peer-to-peer network structure. 
In this network, every participant operates on an equal footing, connecting directly with others without the intermediation of any central servers. 
This architecture not only promotes the seamless sharing and storage of data but also ensures that transactions involving BDTCOIN can be executed 
smoothly and efficiently between parties.


BDTCOIN is crafted to serve both private and public sectors, catering to a broad spectrum of applications. 
For individual users, BDTCOIN provides a high level of privacy, ensuring that personal transactions remain confidential and secure from surveillance. 
In the public domain, BDTCOIN can revolutionize how governmental and non-governmental organizations handle transactions by reducing bureaucratic 
delays and increasing transparency where necessary. This dual utility makes BDTCOIN a versatile digital currency, 
designed to meet diverse needs and adapt to various scenarios,enhancing the efficiency and security of digital exchanges across different spheres of usage.

For more information, as well as an immediately usable, binary version of
the Bdtcoin Core software, see the
[original whitepaper](https://bdtcoin.org/bdtcoin.pdf).

License
-------

Bdtcoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
------------------- 

The `master` branch is regularly built (see `doc/build-*.md` for instructions) and tested, but it is not guaranteed to be
completely stable. [Tags](https://github.com/bdtchain/bdtcoin/tags) are created
regularly from release branches to indicate new official, stable release versions of Bdtcoin Core.

The https://github.com/bdtchain/gui.git repository is used exclusively for the
development of the GUI. Its master branch is identical in all monotree
repositories. Release branches and tags do not exist, so please do not fork
that repository unless it is for development reasons.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md)
and useful hints for developers can be found in [doc/developer-notes.md](doc/developer-notes.md).

Testing
-------

In the dynamic world of software development, particularly within projects that are critical to security, 
the stages of testing and code review represent significant bottlenecks. These phases are crucial as 
ensure the robustness and security of the software, especially when the codebase can influence financial 
outcomes significantly. As such, it's not uncommon to find a scenario where the influx of pull requests 
surpasses the capacity for timely review and testing.

Our development process frequently encounters a higher volume of pull requests than our current resources 
can thoroughly evaluate on short notice. This accumulation not only slows down the development cycle but 
also increases the risk of oversight in a landscape where even minor errors can lead to substantial financial repercussions.

To address this challenge, we urge patience and a collaborative spirit within our community. 
Engaging with the project doesn’t only have to involve submitting your own pull requests. 
An equally valuable contribution is testing and reviewing the submissions of your peers. 
By actively participating in these critical review processes, you can accelerate development timelines, 
enhance the overall quality of the software, and mitigate potential security risks.

This approach not only helps distribute the workload more evenly but also fosters a culture of mutual support and continuous learning. 
Every developer, regardless of their role, gains a deeper understanding of the codebase and a greater appreciation of the project’s complexity and security needs.

Therefore, if you find yourself waiting for feedback on your submission, consider using that time to assist others with theirs. 
This proactive involvement is vital in propelling our project forward, ensuring we maintain the highest standards of 
security and functionality without compromising on the speed of development. Remember, in a project of this scale and importance,
every contribution towards testing and reviewing helps safeguard the integrity of the software and the financial assets it may influence.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and macOS, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

Translations
------------

Translations are periodically pulled from Transifex and merged into the git repository. See the
[translation process](doc/translation_process.md) for details on how this works.

**Important**: We do not accept translation changes as GitHub pull requests because the next
pull from Transifex would automatically overwrite them again.
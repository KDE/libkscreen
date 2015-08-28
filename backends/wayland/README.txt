
Design of libkscreen's Wayland backend

WaylandBackend creates a global static internal config, available through
WaylandBackend::internalConfig(). WaylandConfig binds to the wl_registry
callbacks and catches wl_output creation and destruction. It passes
wl_output creation and removal on to WB::internalConfig() to handle its
internal data representation as WaylandOutput. WaylandOutput binds to
wl_output's callback, and gets notified of geometry and modes, including
changes. WaylandOutput administrates the internal representation of these
objects, and invokes the global notifier, which then runs the pointers it
holds through the updateK* methods in Wayland{Screen,Output,...}.

KScreen:{Screen,Output,Edid,Mode} objects are created from the internal
representation as requested (usually triggered by the creation of a
KScreen::Config object through KScreen::Config::current()). As with other
backends, the objects which are handed out to the lib's user are expected
to be deleted by the user, the backend only takes ownership of its internal
data representation objects.

Note:
In the Wayland backend, we're using the uint32_t name parameter passed by
the callbacks as ids for output. This eases administration, while providing
a consistent set of ids. It means that we somehow have to fit the uint32 in
the int field of libkscreen's APIs. This seems only a potential issue, and
only applies to 32bit systems. Still thinking about this (potential,remote)
problem. This implementation detail should not be seen as an API promise, it
is pure coincidence and is likely to break code assuming it.

                                                            <sebas@kde.org>



ScreenManagement protocol can be used for screen configuration.
It  lists available outputs and allows to change them

- provide info about all outputs, per output:
    - id (int, unique and only handed out by kwin)
    - edid: eisaId, monitorName, serialNumber, physicalSizeX, physicalSizeY
    - primary
    - modes: sizeX, sizeY, refreshRate
    - resolution: resX, resY
    - position: posX, posY
    - orientation: degree
    - enabled: bool
- allows to set, per output:
    - set enabled
    - set mode
    - set orientation
    - set position
    - set primary

- keep protocol flat for simplicity, use id to tell which output this signal refers to
- or: create distinct subinterfaces wrapping our output (seems unnecessarily complex)

server:
    - wayland_server.cpp gets a ScreenManagement interface
    - initOutputs also triggers events in the screenmanagement interface
    - announces the outputs on the ScreenManagementInterface when bound
    - is responsible for state changes through backends
    - decides what to do with the information

client:
    - connects to server
    - receives the info about all the outputs
    - ignores wl_output otherwise
    - can set properties of outputs (enable/disable, mode, position, etc.)

why?
- keeps everything in one place
- one interface for a specific purpose
- bi-directional, so defined config interface, no config writing/picking up etc.
- one place to housekeep ids
- interface is purpose-made, not trying to shoe-horn kscreen in and around wl_output
- server really seems to be the right place to keep state and logic
- makes concertation less complex, since there's exactly one place to handle state, like mode, on/off, etc.




TODO

- remove serializeConfigMinimal
- discuss with mgraesslin wether to move to kwaylandintegration or not
    - disconnected outputs protocol easier to share?
- port writeConfig to KConfig

- output rotation needs reading and writing code + unit tests
- output scale needs reading and writing code + unit tests
- waylandserver picks up config changes
- autotest for server roundtrip with new config
- WaylandScreen takes aggregate size of outputs
- create Edid from output-internal information (which?)
- update Output object
- update Screen object

DONE

o Add modes to configwriter
o Add more output data to configwriter
o autotest for outputs
o autotest for modes
o Make sure config is updated on server shutdown
o handle callback's Done signal? how to trigger? >> interfacesAnnounced()?
o create Modes from wl_output callback
o verify and fix m_outputMap
o Watch for server appearing after config is initialized (out of scope)
o Make case where server is started before anything else happens work
o try to keep wl_output* out of the API (except for WaylandOutput creation)
o properly from wl_output callback
o check with mgraesslin if GPL->LGPL for bits taken from wayland_backend.cpp is OK
o delete wl_* objects in destructors


Disconnected screens protocol in kwayland[sebas/kwin]:


Protocol:
---------

o done references no longer existing "get_disabled_outputs"
o event names are camelcase, Wayland way is underscore

Autotests:
---------
o extend registry test

Server:
------
o do we want the name "KWin" in class name and file name? We don't do that for org_kde_kwin_idle and org_kde_kwin_fake_input
o why private Q_SLOTS done? Why a private method in the public interface?
o edid as QByteArray instead of QString?
o struct DisabledOutput defined but not used anywhere in the public API
o implementation used both Q_FOREACH and new C++11 for each loop, please pick one

Client:
-------
o why org_kde_kwin_screen_management *screen_management()? Aren't the cast operators sufficient?
o misses the release and destroy methods
o why do the signals not carry the DisabledOutput?
* why allow the setters in DisabledOutput? Why should a user of DisabledOutput be allowed to change them? If one wants to allow setting the values, it's missing changed signals
o why is the ctor of DisabledOutput public? Why would one want to construct it? I would friend it with KWinScreenManagement


* then in the signal carry a pointer to a created object of the Output wrapper and on disappeared as well, that might make it easier for a user to track the list of outputs

o merge in registry changes from master
o on client side I suggest to add a small wrapper for the Output information and
o on the server side I think you reversed the meaning of the created class. Normally a "request" ends up as a Q_SIGNAL on the server class, an event ends up as a method, sebas: e.g. KWin would have to call a method when a new output "appeared", which then triggers the event, on the other hand a request ends in the static callback which then needs to notify somehow KWin, e.g. through a signal
o I don't see a need for the request get_disabled_outputs, when an interface gets connected the server starts to emit the state, that is announces all the outputs.  you don't have to care - you connect to the interface and state changes will be emitted, just like everywhere else
o concerning the syncing: do it like in the output interface: emit all "appeared" on connection and then send the done once finished, if there are none from the start, just send done directly
o the manufactorer and model doesn't make so much sense in the server's Private, that sounds more like a list is needed
o rename sync -> done
o rename protocol to org_kde_kwin_screen_management

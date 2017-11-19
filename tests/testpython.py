import gi

gi.require_version('DMAP', '4.0')
gi.require_version('DAAP', '4.0')

from gi.repository import GObject
from gi.repository import GLib
from gi.repository import DMAP
from gi.repository import DAAP

class PyDMAPDb(GObject.GObject, DMAP.Db):
    db = {}
    nextId = 0

    def do_foreach(self, func, user_data=None):
        for id in self.db.copy():
            func (id, self.db[id])

    def do_add(self, record):
        id           = self.nextId
        self.db[id]  = record
        self.nextId += 1
        return id

    def do_count(self):
        return len(self.db);

    def do_lookup_by_id(self, id):
        return self.db[id]

    def __init__(self):
        super(PyDMAPDb, self).__init__()

class PyDAAPRecord(GObject.GObject, DAAP.Record, DMAP.Record):
    location    = GObject.property(type=GObject.TYPE_STRING,  default=None)
    title       = GObject.property(type=GObject.TYPE_STRING,  default=None)
    songalbum   = GObject.property(type=GObject.TYPE_STRING,  default=None)
    sort_album  = GObject.property(type=GObject.TYPE_STRING,  default=None)
    songartist  = GObject.property(type=GObject.TYPE_STRING,  default=None)
    sort_artist = GObject.property(type=GObject.TYPE_STRING,  default=None)
    songgenre   = GObject.property(type=GObject.TYPE_STRING,  default=None)
    format      = GObject.property(type=GObject.TYPE_STRING,  default=None)
    rating      = GObject.property(type=GObject.TYPE_INT,     default=0)
    filesize    = GObject.property(type=GObject.TYPE_UINT64,  default=0)
    duration    = GObject.property(type=GObject.TYPE_INT,     default=0)
    track       = GObject.property(type=GObject.TYPE_INT,     default=0)
    year        = GObject.property(type=GObject.TYPE_INT,     default=0)
    firstseen   = GObject.property(type=GObject.TYPE_INT,     default=0)
    mtime       = GObject.property(type=GObject.TYPE_INT,     default=0)
    disc        = GObject.property(type=GObject.TYPE_INT,     default=0)
    bitrate     = GObject.property(type=GObject.TYPE_INT,     default=0)
    has_video   = GObject.property(type=GObject.TYPE_BOOLEAN, default=0)
    mediakind   = GObject.property(type=DMAP.MediaKind,       default=DMAP.MediaKind.MUSIC)
    songalbumid = GObject.property(type=GObject.TYPE_INT64,   default=0)
    hash        = GObject.property(type=GLib.Array,           default=None)

    def __init__(self):
        super(PyDAAPRecord, self).__init__()

class PyDAAPRecordFactory(GObject.GObject, DMAP.RecordFactory):
    def do_create (self, user_data=None):
        return PyDAAPRecord()

    def __init__(self):
        super(PyDAAPRecordFactory, self).__init__()

class PyDMAPContainerRecord(GObject.GObject, DMAP.ContainerRecord):
    name = GObject.property(type=GObject.TYPE_STRING, default='Test')
    entries = PyDMAPDb()

    def do_get_id(self):
        return 2

    def do_add_entry(self):
        pass

    def do_get_entry_count(self):
        return 1

    def do_get_entries(self):
        return self.entries

    def __init__(self):
        super(PyDMAPContainerRecord, self).__init__()

class PyDMAPContainerDb(GObject.GObject, DMAP.ContainerDb):
    record = PyDMAPContainerRecord()

    def do_add(self, record):
        pass

    def do_count(self):
        return 1;

    def do_foreach(self, func, user_data=None):
        func (1, self.record)

    def do_lookup_by_id(self, id):
        return self.record

    def __init__(self):
        super(PyDMAPContainerDb, self).__init__()

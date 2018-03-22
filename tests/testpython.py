import gi

gi.require_version('Dmap', '4.0')

from gi.repository import GObject
from gi.repository import GLib
from gi.repository import Dmap

class PyDmapDb(GObject.GObject, Dmap.Db):
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
        super(PyDmapDb, self).__init__()

class PyDaapRecord(GObject.GObject, Dmap.AvRecord, Dmap.Record):
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
    mediakind   = GObject.property(type=Dmap.MediaKind,       default=Dmap.MediaKind.MUSIC)
    songalbumid = GObject.property(type=GObject.TYPE_INT64,   default=0)
    hash        = GObject.property(type=GLib.Array,           default=None)

    def __init__(self):
        super(PyDaapRecord, self).__init__()

class PyDaapRecordFactory(GObject.GObject, Dmap.RecordFactory):
    def do_create (self, user_data=None):
        return PyDaapRecord()

    def __init__(self):
        super(PyDaapRecordFactory, self).__init__()

class PyDmapContainerRecord(GObject.GObject, Dmap.ContainerRecord):
    name = GObject.property(type=GObject.TYPE_STRING, default='Test')
    entries = PyDmapDb()

    def do_get_id(self):
        return 2

    def do_add_entry(self):
        pass

    def do_get_entry_count(self):
        return 1

    def do_get_entries(self):
        return self.entries

    def __init__(self):
        super(PyDmapContainerRecord, self).__init__()

class PyDmapContainerDb(GObject.GObject, Dmap.ContainerDb):
    record = PyDmapContainerRecord()

    def do_add(self, record):
        pass

    def do_count(self):
        return 1;

    def do_foreach(self, func, user_data=None):
        func (1, self.record)

    def do_lookup_by_id(self, id):
        return self.record

    def __init__(self):
        super(PyDmapContainerDb, self).__init__()

import lldb


def _read_string(process, ptr, count):
    err = lldb.SBError()
    raw = process.ReadMemory(ptr, count, err)
    if err.Success():
        return '"%s"' % raw.decode("utf-8", errors="replace")
    return '"<unreadable>"'


def _slice_parts(val):
    raw = val.GetNonSyntheticValue()
    data = raw.GetChildMemberWithName("data")
    count = raw.GetChildMemberWithName("count").GetValueAsSigned()
    return data.GetValueAsUnsigned(), count


def utf8str_summary(val, _dict):
    data_addr, count = _slice_parts(val)
    if data_addr == 0 or count <= 0:
        return "<empty>"
    return _read_string(val.GetProcess(), data_addr, count)


def Slice_summary(val, _dict):
    data_addr, count = _slice_parts(val)
    if data_addr == 0 or count <= 0:
        return "<empty>"
    return "{ count=%d }" % count


def List_summary(val, _dict):
    raw = val.GetNonSyntheticValue()
    data_addr = raw.GetChildMemberWithName("data").GetValueAsUnsigned()
    count = raw.GetChildMemberWithName("count").GetValueAsSigned()
    capacity = raw.GetChildMemberWithName("capacity").GetValueAsSigned()
    if data_addr == 0 or count <= 0:
        return "<empty>"
    return "{ count=%d, capacity=%d }" % (count, capacity)


class SliceSynthProvider:
    def __init__(self, val, _dict):
        self.val = val
        self.data = None
        self.elem_type = None

    def update(self):
        raw = self.val.GetNonSyntheticValue()
        self.data = raw.GetChildMemberWithName("data")
        self.elem_type = self.data.GetType().GetPointeeType()

    def _get_count(self):
        return (
            self.val.GetNonSyntheticValue()
            .GetChildMemberWithName("count")
            .GetValueAsSigned()
        )

    def num_children(self):
        count = self._get_count()
        return count if 0 < count < 65536 else 0

    def get_child_at_index(self, idx):
        count = self._get_count()
        if idx < 0 or idx >= count or self.data is None:
            return None
        offset = idx * self.elem_type.GetByteSize()
        return self.data.CreateChildAtOffset("[%d]" % idx, offset, self.elem_type)

    def get_child_index(self, name):
        try:
            return int(name.lstrip("[").rstrip("]"))
        except:
            return -1

    def has_children(self):
        return self._get_count() > 0


class ListSynthProvider(SliceSynthProvider):
    pass


def __lldb_init_module(debugger, _dict):
    cat = "col"
    debugger.HandleCommand("type category define %s" % cat)

    # generic slices
    debugger.HandleCommand(
        'type summary add -w %s -x -F formatters.Slice_summary "^COL_Slice_"' % cat
    )
    debugger.HandleCommand(
        'type synthetic add -w %s -x -l formatters.SliceSynthProvider "^COL_Slice_"'
        % cat
    )

    # utf8str slice
    debugger.HandleCommand(
        'type summary add -w %s -F formatters.utf8str_summary "COL_Slice_u8"' % cat
    )
    debugger.HandleCommand(
        'type summary add -w %s -F formatters.utf8str_summary "utf8str"' % cat
    )

    # lists
    debugger.HandleCommand(
        'type summary add -w %s -x -F formatters.List_summary "^COL_List_"' % cat
    )
    debugger.HandleCommand(
        'type synthetic add -w %s -x -l formatters.ListSynthProvider "^COL_List_"' % cat
    )

    debugger.HandleCommand("type category enable %s" % cat)
    print("[formatters] loaded")

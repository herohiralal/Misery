import lldb


def _read_string(process, ptr, count):
    err = lldb.SBError()
    raw = process.ReadMemory(ptr, count, err)
    if err.Success():
        return '"%s"' % raw.decode("utf-8", errors="replace")
    return "<unreadable>"


def _slice_parts(val):
    """Return (data_addr, count) reading raw struct fields, bypassing synth."""
    raw = val.GetNonSyntheticValue()
    data = raw.GetChildMemberWithName("data")
    count = raw.GetChildMemberWithName("count").GetValueAsUnsigned()
    return data.GetValueAsUnsigned(), count


def CString_summary(val, _dict):
    data = val.GetNonSyntheticValue().GetChildMemberWithName("data")
    if data.GetValueAsUnsigned() == 0:
        return "<null>"
    return data.GetSummary() or "<null>"


def String_summary(val, _dict):
    raw = val.GetNonSyntheticValue()
    sl = raw.GetChildMemberWithName("slice").GetNonSyntheticValue()
    data_addr, count = _slice_parts(sl)
    if data_addr == 0 or count == 0:
        return "<empty>"
    return _read_string(val.GetProcess(), data_addr, count)


def SliceChar_summary(val, _dict):
    data_addr, count = _slice_parts(val)
    if data_addr == 0 or count == 0:
        return "<empty>"
    return _read_string(val.GetProcess(), data_addr, count)


def Slice_summary(val, _dict):
    data_addr, count = _slice_parts(val)
    if data_addr == 0 or count == 0:
        return "<empty>"
    return "{ count=%d }" % count


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
            .GetValueAsUnsigned()
        )

    def num_children(self):
        count = self._get_count()
        return count if count < 65536 else 0

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


def List_summary(val, _dict):
    raw = val.GetNonSyntheticValue()
    data = raw.GetChildMemberWithName("data").GetValueAsUnsigned()
    count = raw.GetChildMemberWithName("count").GetValueAsUnsigned()
    capacity = raw.GetChildMemberWithName("capacity").GetValueAsUnsigned()
    if data == 0 or count == 0:
        return "<empty>"
    return "{ count=%d, capacity=%d }" % (count, capacity)


class ListSynthProvider(SliceSynthProvider):
    pass


def __lldb_init_module(debugger, _dict):
    cat = "misery_core"
    debugger.HandleCommand("type category define %s" % cat)

    # CString
    debugger.HandleCommand(
        'type summary add -w %s -F formatters.CString_summary "CString"' % cat
    )

    # String
    debugger.HandleCommand(
        'type summary add -w %s -F formatters.String_summary "String"' % cat
    )

    # Slice<char> and Slice<unsigned char> — must be before the generic regex
    debugger.HandleCommand(
        'type summary add -w %s -F formatters.SliceChar_summary "Slice<char>"' % cat
    )
    debugger.HandleCommand(
        'type summary add -w %s -F formatters.SliceChar_summary "Slice<unsigned char>"'
        % cat
    )

    # Slice<T> generic — summary + array expansion
    debugger.HandleCommand(
        'type summary add -w %s -x -F formatters.Slice_summary "Slice<.+>"' % cat
    )
    debugger.HandleCommand(
        'type synthetic add -w %s -x -l formatters.SliceSynthProvider "Slice<.+>"' % cat
    )

    # List<T> — summary + array expansion
    debugger.HandleCommand(
        'type summary add -w %s -x -F formatters.List_summary "List<.+>"' % cat
    )
    debugger.HandleCommand(
        'type synthetic add -w %s -x -l formatters.ListSynthProvider "List<.+>"' % cat
    )

    debugger.HandleCommand("type category enable %s" % cat)
    print("[formatters] custom types loaded")

import base64
import hashlib

def rehash_state(state, append_dict=True):
    ret = None
    # Create an MD5 hasher object
    h = hashlib.md5()
    h.update('ghw')
    if isinstance(state, list):
        for x in state:
            h.update(rehash_state(x, append_dict))
    elif isinstance(state, dict):
        for key, value in state:
            if key == '_id':
                continue
            h.update(key)
            h.update(rehash_state(value, False))
        if append_dict:
            ret = base64.b64encode(h.digest())[0:8]
            h['_id'] = ret
    else:
        h.update(state)
    if not ret:
        ret = base64.b64encode(h.digest())[0:8]
    return ret

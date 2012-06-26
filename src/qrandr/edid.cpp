
    const QByteArray EDID_FIXED_HEADER("\x00\xFF\xFF\xFF\xFF\xFF\xFF");
    const unsigned int EDID_VERSION_1 = 18;
    const unsigned int EDID_VERSION_2 = 19;
    const unsigned int EDID_VERSION_1_3_SIZE = 128;
    const unsigned int EDID_DESCRIPTOR_START = 54;
    const unsigned int EDID_DESCRIPTOR_TYPE = 3;
    const char EDID_DESCRIPTOR_NAME_TYPE = 0xFC;
    const unsigned int EDID_DESCRIPTOR_NAME_SKIP = 2;
    const unsigned int EDID_DESCRIPTOR_NAME_END = 17;
    const unsigned int EDID_DESCRIPTOR_LENGTH = 18;
    const unsigned int EDID_DESCRIPTORS = 4;
    const unsigned int EDID_DESCRIPTOR_MAX_NAME = 13;

void Output::fetchEdidData()
{
    qDebug() << "Fetching Edid Data...";
    if (!isConnected()) {
        qDebug() << "Monitor not connected";
        return;
    }

    QByteArray edid = fetchProps("EDID");
    if (edid.isEmpty()) {
        edid = fetchProps("EDID_DATA");
    }

    qDebug() << edid.toBase64();

    fetchEdidName(edid);
    exit(1);
}
QByteArray Output::fetchProps(const QByteArray& name)
{
    Atom atom;

    atom = XInternAtom (QX11Info::display(), name, FALSE);

    unsigned char *prop;
    int actual_format;
    unsigned long nitems, bytes_after;
    Atom actual_type;

    XRRGetOutputProperty (QX11Info::display(), m_id, atom,
            0, 100, False, False,
            AnyPropertyType,
            &actual_type, &actual_format,
            &nitems, &bytes_after, &prop);

    if (actual_type != XA_INTEGER || actual_format != 8)
    {
        XFree (prop);
        return QByteArray();
    }

    QByteArray edid(reinterpret_cast<const char*>(prop), nitems);
    XFree (prop);

    return edid;
}

static int
get_bits (int in, int begin, int end)
{
    int mask = (1 << (end - begin + 1)) - 1;

    return (in >> begin) & mask;
}

void Output::fetchEdidName(const QByteArray &edid)
{
    if (edid.isEmpty()) {
        qDebug() << "Edid is empty";
        return;
    }
    if (!edid.startsWith("\x00\xff\xff\xff\xff\xff\xff\x00")) {
        qDebug() << "Invalid edid data";
        qDebug() << edid.toHex();
        return;
    }

    qDebug() << "Edid seems correct";
    if (!edid.startsWith(EDID_FIXED_HEADER)) {
        qDebug() << "EDID doesn't start with known header";
        return;
    }
    if (!edid.size() == EDID_VERSION_1_3_SIZE) {
        qDebug() << "EDID doesn't have the right size";
        return;
    }
    int v1 = edid.at(EDID_VERSION_1);
    int v2 = edid.at(EDID_VERSION_2);

    QString version;
    version.sprintf("%i.%i",v1, v2);
    qDebug() << "Version: " << version;
    if (version != "1.3") {
        qDebug() << "Unknown version";
        return;
    }


    /* Manufacturer Code */
    char manufacturer_code[4];
    manufacturer_code[0]  = get_bits (edid[0x08], 2, 6);
    manufacturer_code[1]  = get_bits (edid[0x08], 0, 1) << 3;
    manufacturer_code[1] |= get_bits (edid[0x09], 5, 7);
    manufacturer_code[2]  = get_bits (edid[0x09], 0, 4);
    manufacturer_code[3]  = '\0';

    manufacturer_code[0] += 'A' - 1;
    manufacturer_code[1] += 'A' - 1;
    manufacturer_code[2] += 'A' - 1;

    qDebug() << manufacturer_code;

    return;

    unsigned int pType = EDID_DESCRIPTOR_START + EDID_DESCRIPTOR_TYPE;
    bool nameDescriptorFound = false;
    for (unsigned int i = 0; i < EDID_DESCRIPTORS; i++) {
        char descType = edid.at(pType);
        if (descType == EDID_DESCRIPTOR_NAME_TYPE) {
            nameDescriptorFound = true;
            break;
        }
        pType +=  EDID_DESCRIPTOR_LENGTH;
    }
    if (!nameDescriptorFound) {
        qDebug() << "Monitor name descriptor not found";
        return;
    }
    QString name;
    unsigned int pName = pType + EDID_DESCRIPTOR_NAME_SKIP;
    for (unsigned int j = 0; j <  EDID_DESCRIPTOR_MAX_NAME; j++) {
        char nameChar = edid.at(pName);
        if (nameChar  != '\n') {
            name += nameChar;
            pName++;
        } else {
            break;
        }
    }
    qDebug() << "Name descriptor found: " << name;
}
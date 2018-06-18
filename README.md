# Stereo Mixer

An LV2 audio plug-in for stereo-signal manipulation.

## How to build

Just run:

```bash
make
```

And you get `stereo-mixer.lv2` directory of the plug-in.

You could for example make symbolic link to it
in `/usr/local/lib64/lv2` directory or just copy it there:

```bash
cp -r stereo-mixer.lv2/ /usr/local/lib64/lv2/
```

You also may want to build _DEBUG_ version of the plug-in:

```bash
make DEBUG=Y
```

# Author

Viacheslav Lotsmanov

# License

[GPLv3](./LICENSE)

import GObject from 'gi://GObject';
import GLib from 'gi://GLib';
import Gio from 'gi://Gio';

import {Extension} from 'resource:///org/gnome/shell/extensions/extension.js';
import {QuickToggle, SystemIndicator} from 'resource:///org/gnome/shell/ui/quickSettings.js';
import * as Main from 'resource:///org/gnome/shell/ui/main.js';

const UNIT = 'hotspot.service';
const SSID = 'fedora-hotspot';

function run(argv, callback) {
    try {
        const proc = Gio.Subprocess.new(
            argv, Gio.SubprocessFlags.STDOUT_PIPE | Gio.SubprocessFlags.STDERR_PIPE);
        proc.communicate_utf8_async(null, null, (p, res) => {
            let out = '';
            try {
                const [, stdout] = p.communicate_utf8_finish(res);
                out = stdout ? stdout.trim() : '';
            } catch (e) {}
            callback(out);
        });
    } catch (e) {
        callback('');
    }
}

const HotspotToggle = GObject.registerClass(
class HotspotToggle extends QuickToggle {
    _init() {
        super._init({
            title: 'Hotspot',
            subtitle: SSID,
            iconName: 'network-wireless-hotspot-symbolic',
            toggleMode: false,
        });

        this._busy = false;
        this.connect('clicked', () => this._onClicked());

        this._timeout = GLib.timeout_add_seconds(GLib.PRIORITY_DEFAULT, 5, () => {
            this._refresh();
            return GLib.SOURCE_CONTINUE;
        });
        this._refresh();
    }

    _refresh() {
        if (this._busy)
            return;
        run(['systemctl', 'is-active', UNIT], out => {
            this.checked = out === 'active';
            this.subtitle = this.checked ? SSID : 'Off';
        });
    }

    _onClicked() {
        if (this._busy)
            return;
        this._busy = true;
        const starting = !this.checked;
        this.subtitle = starting ? 'Starting…' : 'Stopping…';
        run(['systemctl', starting ? 'start' : 'stop', UNIT], () => {
            this._busy = false;
            this._refresh();
        });
    }

    destroy() {
        if (this._timeout) {
            GLib.source_remove(this._timeout);
            this._timeout = null;
        }
        super.destroy();
    }
});

const HotspotIndicator = GObject.registerClass(
class HotspotIndicator extends SystemIndicator {
    _init() {
        super._init();
        this._indicator = this._addIndicator();
        this._indicator.iconName = 'network-wireless-hotspot-symbolic';

        this._toggle = new HotspotToggle();
        this._toggle.bind_property('checked', this._indicator, 'visible',
            GObject.BindingFlags.SYNC_CREATE);

        this.quickSettingsItems.push(this._toggle);
    }
});

export default class HotspotExtension extends Extension {
    enable() {
        this._indicator = new HotspotIndicator();
        Main.panel.statusArea.quickSettings.addExternalIndicator(this._indicator);
    }

    disable() {
        this._indicator.quickSettingsItems.forEach(item => item.destroy());
        this._indicator.destroy();
        this._indicator = null;
    }
}

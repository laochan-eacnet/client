import { ref, type Ref } from "vue";
import { GameClass } from "./gameClass";

export interface DDRConfig {
}

export class DDR extends GameClass {
    private _config: Ref<DDRConfig | undefined> = ref(undefined);
    private _dirty: boolean = false;

    get gameIndex(): number {
        return 3;
    }

    get name(): string {
        return 'DDR GrandPrix';
    }

    get className(): string {
        return 'ddr';
    }

    get config() {
        return this._config;
    }

    async resetConfig() {
        this._config.value = {}
        this._dirty = true;
    }

    async saveConfig() {
        const path = this.configPath;
        if (!path) {
            return;
        }

        const result = JSON.stringify(this._config.value);
        await window.laochan.writeFile(path, result);
        this._dirty = false;
    }

    async loadConfig() {
        const path = this.configPath;
        if (!path) {
            return;
        }

        const configJson = await window.laochan.readFile(path);

        if (configJson) {
            this._config.value = JSON.parse(configJson);
        }

        if (!this._config.value) {
            await this.resetConfig();
        }

        if (this._dirty) {
            await this.saveConfig();
        }
    }

    async applyConfig() {
        const config = this._config.value;
        if (!config) {
            return;
        }

        const meta = await window.laochan.detectGameInstall(this.gameIndex);
        const settingsPath = meta.resource_path + '/config/settings_cf.xml';

        const settings = await window.laochan.readFile(settingsPath);
        if (!settings) {
            return;
        }

        const parser = new DOMParser();
        const settingDoc = parser.parseFromString(settings, "application/xml");
        
        const args: string[] = ['ddr-konaste.exe'];

        settingDoc.querySelectorAll('KeyValuePair').forEach(kvp => {
            const ke = kvp.querySelector('Key');
            const ve = kvp.querySelector('Value');

            if (!ke || !ve) {
                return;
            }

            const key = ke.querySelector('string')?.innerHTML;
            const value = ve.querySelector('string')?.innerHTML;

            if (!key || !value) {
                return;
            }
            
            if (key === 'DisplaySettings') {
                args.push('--display' + value);
            } else if (key === "WindowSettings") {
                if (value == '2') {
                    args.push('--borderless');
                } else if (value == '1') {
                    args.push('--fullscreen');
                }
            } else if (key === 'FPSSettings') {
                args.push(value === '1' ? '--fps120' : '--fps60');
            }
        });

        window.laochan.setParam('DDR_LAUNCH_ARGS', args.join(' '));
    }
};

export const ddr = new DDR();

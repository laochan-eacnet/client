import { ref, type Ref } from "vue";
import { launcher, VersionState } from "./launcher";
import { GameClass } from "./gameClass";

export interface SDVXConfig {
}

export class SDVX extends GameClass {
    private _config: Ref<SDVXConfig | undefined> = ref(undefined);
    private _dirty: boolean = false;
   
    get gameIndex(): number {
        return 1;
    }

    get name(): string {
        return 'SDVX Exceed Gear';
    }

    get className(): string {
        return 'sdvx';
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
    }
};

export const sdvx = new SDVX();

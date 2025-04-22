import { ref, type Ref } from "vue";
import { launcher, VersionState } from "./launcher";
import type { RefSymbol } from "@vue/reactivity";
import { faL } from "@fortawesome/free-solid-svg-icons";
import { GameClass } from "./gameClass";

export interface GITADORAConfig {
}

export class GITADORA extends GameClass {
    private _config: Ref<GITADORAConfig | undefined> = ref(undefined);
    private _dirty: boolean = false;

    get gameIndex(): number {
        return 2;
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

    async startGf() {
        await window.laochan.setParam('GITADORA_LAUNCH_ARGS', ' -gf');
        return this.start();
    }

    startDm() {
        return this.start();
    }
};

export const gitadora = new GITADORA();

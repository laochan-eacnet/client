import { ref, type Ref } from "vue";
import { launcher } from "./launcher";

export interface GITADORAConfig {
}

export class GITADORA {
    private _config: Ref<GITADORAConfig | undefined> = ref(undefined);
    private _dirty: boolean = false;

    get config() {
        return this._config;
    }

    installed() {
        return !!window.laochan.ctx.gameInfos.value[2].installed;
    }
    
    get installPath() {
        if (!this.installed()) {
            return;
        }

        const [installPath] = window.laochan.ctx.gameInfos.value[2].install_path;
        return installPath;
    }

    get configPath() {
        const installPath = this.installPath;
        if (!installPath) {
            return;
        }

        return installPath + 'laochan-config.json';
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

    async start() {
        await this.loadConfig();
        await window.laochan.setGame(2);

        await this.applyConfig();
        await launcher.applyConfig();

        window.laochan.close();
    }

    async settings() {
        const installPath = this.installPath;
        if (!installPath) {
            return;
        }

        window.laochan.shellExecute(installPath + '\\launcher\\modules\\settings.exe');
    }

    async updater() {
        const installPath = this.installPath;
        if (!installPath) {
            return;
        }

        window.laochan.shellExecute(installPath + '\\launcher\\modules\\updater.exe', '-t DUMMY');
    }
};

export const gitadora = new GITADORA();

import { ref } from "vue";

export interface LauncherConfig {
    token: string;
    serverUrl: string;
    enableConsole: boolean;
    enableSteamOverlay: boolean;
}

export class Launcher {
    private _config: Ref<LauncherConfig | undefined> = ref(undefined);
    private _dirty: boolean = false;

    get config() {
        return this._config;
    }

    async resetToken() {
        this._config.value!.token = await window.laochan.uuid();
        this._dirty = true;
    }

    resetServerUrl() {
        this._config.value!.serverUrl = 'http://laochan.ugreen.sbs/';
        this._dirty = true;
    }

    resetEnableConsole() {
        this._config.value!.enableConsole = false;
        this._dirty = true;
    }

    resetEnableSteamOverlay() {
        this._config.value!.enableSteamOverlay = true;
        this._dirty = true;
    }

    async saveConfig() {
        const result =  JSON.stringify(this._config.value);
        await window.laochan.writeFile('laochan-config.json', result);
        this._dirty = false;
    } 

    async loadConfig() {
        const configJson = await window.laochan.readFile('laochan-config.json');
        if (configJson) {
            this._config.value = JSON.parse(configJson);
        }

        if (!configJson) {
            this._config.value = {} as LauncherConfig;
        }

        if (!this._config.value!.token) {
            await this.resetToken();
        }

        if (!this._config.value!.serverUrl) {
            this.resetServerUrl();
        }

        if (!this._config.value!.enableConsole === undefined) {
            this.resetEnableConsole();
        }

        if (!this._config.value!.enableSteamOverlay === undefined) {
            this.resetEnableSteamOverlay();
        }

        if (this._dirty) 
            this.saveConfig();
    }

    async applyConfig() {
        if (!this._config.value)
            return;

        await window.laochan.setParam('LAOCHAN_TOKEN', this._config.value.token);
        await window.laochan.setParam('LAOCHAN_SERVER_URL', this._config.value.serverUrl);
        await window.laochan.setParam('LAOCHAN_ENABLE_CONSOLE', JSON.stringify(+this._config.value.enableConsole));
        await window.laochan.setParam('LAOCHAN_ENABLE_STEAM_OVERLAY', JSON.stringify(+this._config.value.enableConsole));
    }
}

export const launcher = new Launcher();

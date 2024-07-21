import { ref, type Ref } from "vue";
import { launcher } from "./launcher";

export enum IIDXDisplayMode {
    Fullscreen = 0,
    Windowed720p = 1,
    Windowed900p = 2,
    Windowed1080p = 3,
}

export enum IIDXSoundMode {
    Wasapi = 0,
    Asio = 1,
}

export interface IIDXConfig {
    displayMode: IIDXDisplayMode;
    soundMode: IIDXSoundMode;
    asioDevice: string;
    useGsm: boolean;
}

export class IIDX {
    private _config: Ref<IIDXConfig | undefined> = ref(undefined);
    private _dirty: boolean = false;

    get config() {
        return this._config;
    }

    get installPath() {
        if (!this.installed()) {
            return;
        }

        const [installPath] = window.laochan.ctx.gamePaths.value[0];
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
        const { devices } = await window.laochan.getAsioDeviceList();

        this._config.value = {
            displayMode: IIDXDisplayMode.Windowed1080p,
            soundMode: IIDXSoundMode.Wasapi,
            asioDevice: devices[0],
            useGsm: true,
        }

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

        if (!configJson || 
            this._config.value?.asioDevice == undefined || 
            this._config.value.displayMode === undefined ||
            this._config.value.soundMode === undefined ||
            this._config.value.useGsm === undefined
        ) {
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

        await Promise.all([
            window.laochan.setParam('IIDX_ASIO_DEVICE', config.asioDevice),
            window.laochan.setParam('IIDX_DISPLAY_MODE', JSON.stringify(config.displayMode)),
            window.laochan.setParam('IIDX_SOUND_DEVICE', JSON.stringify(config.soundMode)),
            window.laochan.setParam('IIDX_USE_GSM', JSON.stringify(+config.useGsm)),
        ]);
    }

    openCustomize() {
        window.laochan.shellExecute('http://laochan.ugreen.sbs/cp');
    }

    installed() {
        return !!window.laochan.ctx.gamePaths.value[0].length;
    }

    async start() {
        await this.loadConfig();
        await window.laochan.setGame(0);

        await this.applyConfig();
        await launcher.applyConfig();

        window.laochan.close();
    }

    async settings() {
        const installPath = this.installPath;
        if (!installPath) {
            return;
        }

        window.laochan.shellExecute(installPath + '\\launcher\\modules\\bm2dx_settings.exe');
    }

    async updater() {
        const installPath = this.installPath;
        if (!installPath) {
            return;
        }

        window.laochan.shellExecute(installPath + '\\launcher\\modules\\bm2dx_updater.exe');
    }
};

export const iidx = new IIDX();

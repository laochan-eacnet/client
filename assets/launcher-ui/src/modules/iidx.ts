import { ref, type Ref } from "vue";
import { launcher, VersionState } from "./launcher";
import dedent from "dedent";
import type { TupleType } from "typescript";

export enum IIDXDisplayMode {
    Fullscreen = 0,
    BorderlessWindowed = 1,
    Windowed = 1,
}

export enum IIDXSoundMode {
    Wasapi = 0,
    Asio = 1,
}

export enum IIDXLanguage {
    Japanese = 0,
    English = 1,
    Korean = 2,
}

export interface IIDXConfig {
    displayMode: IIDXDisplayMode;
    resolution: {
        w: number;
        h: number;
    };
    soundMode: IIDXSoundMode;
    asioDevice: string;
    useGsm: boolean;
    language: IIDXLanguage;
}

export class IIDX {
    private _config: Ref<IIDXConfig | undefined> = ref(undefined);
    private _dirty: boolean = false;

    get config() {
        return this._config;
    }

    installed() {
        return !!window.laochan.ctx.gameInfos.value[0].installed;
    }

    get installPath() {
        if (!this.installed()) {
            return;
        }

        const installPath = window.laochan.ctx.gameInfos.value[0].install_path;
        return installPath;
    }

    get configPath() {
        const installPath = this.installPath;
        if (!installPath) {
            return;
        }

        return installPath + 'laochan-config.json';
    }

    checkVersion(): VersionState {
        if (!this.installed()) {
            return VersionState.Unknown;
        }
        const installVersion = window.laochan.ctx.gameInfos.value[0].game_module_version;
        const targetVersion = window.laochan.ctx.gameInfos.value[0].game_module_target_version;
        const installVersionNum = Number.parseInt(installVersion.split(":")[4]);
        const targetVersionNum = Number.parseInt(targetVersion.split(":")[4]);
        if (installVersionNum > targetVersionNum) {
            return VersionState.Need2UpdateLauncher;
        }
        else if (installVersionNum < targetVersionNum) {
            return VersionState.Need2UpdateGame;
        }
        else {
            return VersionState.Normal
        }
    }

    async resetConfig() {
        const { devices } = await window.laochan.getAsioDeviceList();

        this._config.value = {
            displayMode: IIDXDisplayMode.BorderlessWindowed,
            resolution: { w: 0, h: 0 },
            soundMode: IIDXSoundMode.Wasapi,
            asioDevice: devices[0],
            useGsm: true,
            language: IIDXLanguage.English,
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

        if (!this._config.value ||
            this._config.value.asioDevice == undefined ||
            this._config.value.displayMode === undefined ||
            this._config.value.resolution === undefined ||
            this._config.value.soundMode === undefined ||
            this._config.value.useGsm === undefined ||
            this._config.value.language === undefined
        ) {
            window.laochan.alert.show('IIDX 设置已被重置, 请前往额外设置重新设置', '#B64040', 2000);
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
            window.laochan.setParam('IIDX_SOUND_MODE', JSON.stringify(config.soundMode)),
            window.laochan.setParam('IIDX_USE_GSM', JSON.stringify(+config.useGsm)),
            window.laochan.setParam('IIDX_LANGUAGE', JSON.stringify(+config.language)),

            window.laochan.setParam('IIDX_RESOLTION_W', JSON.stringify(config.resolution.w)),
            window.laochan.setParam('IIDX_RESOLTION_H', JSON.stringify(config.resolution.h)),
        ]);
    }

    openCustomize() {
        window.laochan.shellExecute('http://laochan.ugreen.sbs/cp');
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

    async generateBat() {
        const config = this._config.value;
        if (!config) {
            return '';
        }

        const selfPath = await window.laochan.selfPath();

        const result = dedent`
            REM generated by Laochan Launcher

            ${await launcher.exportBatParams()}

            REM IIDX Options
            SET IIDX_ASIO_DEVICE="${config.asioDevice}"
            SET IIDX_DISPLAY_MODE=${config.displayMode}
            SET IIDX_SOUND_MODE=${config.soundMode}
            SET IIDX_USE_GSM=${+config.useGsm}
            SET IIDX_LANGUAGE=${+config.language}
            SET IIDX_RESOLTION_W=${config.resolution.w}
            SET IIDX_RESOLTION_H=${config.resolution.h}

            start "" "${selfPath}" "-iidx"
        `;

        const handle = await window.showSaveFilePicker({
            suggestedName: 'laochan-iidx.bat',
            startIn: 'desktop',
            types: [{
                description: 'Batch File',
                accept: { 'application/bat': ['.bat', '.cmd'] }
            }]
        })

        const writable = await handle.createWritable();
        await writable.write(result);
        await writable.close();
    }
};

export const iidx = new IIDX();

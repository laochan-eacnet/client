import { ref, type Ref } from "vue";
import { launcher, VersionState } from "./launcher";
import type { RefSymbol } from "@vue/reactivity";
import { faL } from "@fortawesome/free-solid-svg-icons";

export interface GITADORAConfig {
}

export class GITADORA {
    private _config: Ref<GITADORAConfig | undefined> = ref(undefined);
    private _dirty: boolean = false;
    public GameMeta: Ref<GameMeta | undefined> = ref(undefined);
    public GameVersionState: Ref<VersionState> = ref(VersionState.Unknown);

    get config() {
        return this._config;
    }

    installed() {
        return this.GameMeta.value?.installed ?? false;
    }

    async UpdateMeta() {
        this.GameMeta.value = await window.laochan.detectGameInstall(2);
        this.GameVersionState.value = this.checkVersion();
    }

    get installPath() {
        if (!this.installed()) {
            return;
        }
        return this.GameMeta.value!.install_path;
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
        const installVersion = this.GameMeta.value!.game_module_version;
        const targetVersion = this.GameMeta.value!.game_module_target_version;
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

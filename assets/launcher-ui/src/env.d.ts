/// <reference types="vite/client" />
interface Ref<T = any> {
    value: T;
}

interface Window {
    saucer: {
        _idc: number;
        _rpc: unknown[];
        window_edge: {
            top: number;
            bottom: number;
            left: number;
            right: number;
        },
        on_message: (msg: string) => Promise<void>;
        start_drag: () => Promise<void>;
        start_resize: (edge: number) => Promise<void>;
        call: <T = unknown>(name: string, params: unknown[]) => Promise<T>;
        _resolve: (id: string, value: unknown) => Promise<void>;
    };
    laochan: {
        close: () => void;
        version: () => Promise<string>;
        minimize: () => void;
        mounted: () => void;
        shellExecute: (file: string, args?: string) => void;
        detectGameInstall: (gameIndex: number) => Promise<string[]>;
        readFile: (path: string) => Promise<string | undefined>;
        writeFile: (path: string, content: string) => Promise<void>;
        uuid: () => Promise<string>;
        setGame: (game: number) => Promise<void>;
        setParam: (key: string, value: string) => Promise<void>;
        getAsioDeviceList: () => Promise<{ devices: string[], avaliable: boolean }>;
        /// 0 = ok
        /// 1 = wasapi device not supports 44100 hz
        /// -1 = failed to enum device
        /// -2 = failed to get default audio device
        /// -3 = failed to active wasapi audio client
        checkWasapiDevice: () => Promise<number>;
        alert: {
            show: (message: string, color: string, timeout: number) => void;
            __cb: ((message: string, color: string, timeout: number) => void) | undefined;
        },
        ctx: {
            gamePaths: Ref<string[][]>;
        },
    },
    chrome: any;
}

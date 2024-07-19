/// <reference types="vite/client" />

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
        minimize: () => void;
        mounted: () => void;
        shellExecute: (file: string, args?: string) => void;
        detectGameInstall: (gameIndex: number) => Promise<string[]>;
        readFile: (path: string) => Promise<string | undefined>;
        writeFile: (path: string, content: string) => Promise<void>;
        uuid: () => Promise<string>;
        ctx: {
            gamePaths: Ref<string[][]>;
        },
    },
    chrome: any;
}

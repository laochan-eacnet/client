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
        call: (name: string, params: unknown[]) => Promise<unknown>;
        _resolve: (id: string, value: unknown) => Promise<void>;
    };
    chrome: any;
}

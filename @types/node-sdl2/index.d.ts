
declare module 'node-sdl2' {
    class window {
        on(event: string, callback: () => void): void;
    }

    namespace App {
        function quit(): void;
    }
}
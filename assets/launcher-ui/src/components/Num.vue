<script setup lang="ts">
//@ts-expect-error ??
import { useSound } from '@vueuse/sound';
import num from '../assets/num.mp3'
import { ref } from 'vue';

const element: Ref<HTMLElement | undefined> = ref(undefined);
let timer: number | undefined = undefined;

const { play } = useSound(num);

window.laochan.num = () => {
    if (!element.value) return;

    clearTimeout(timer);

    element.value.classList.remove('anim');
    element.value.classList.add('anim');
    play();

    timer = setTimeout(() => {
        if (!element.value) return;

        element.value.classList.remove('anim');

        clearTimeout(timer);
        timer = undefined;
    }, 1000);
};
</script>

<template>
    <div ref="element" class="num-bg">
        <div class="num"></div>
    </div>
</template>

<style scoped>
.num-bg {
    display: none;
    position: fixed;
    z-index: 50;
    top: 0;
    left: 0;
    width: 100vw;
    height: 100vh;
}

.num-bg.anim {
    display: block;
}

@keyframes num {
    0% {
        opacity: 1;
        transform: scale(50);
    }

    40% {
        opacity: 1;
        transform: scale(1);
    }

    80%{
        opacity: 1;
    }

    100% {
        opacity: 0;
    }
}

.num {
    background-image: url(../assets/num.png);
    width: 100%;
    height: 100%;
    opacity: 0;
}

.num-bg.anim>.num {
    animation: num 0.6s ease;
}
</style>

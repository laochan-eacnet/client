<script setup lang="ts">
import { onMounted, ref, type Ref } from 'vue';
import IconClose from '@/components/icons/IconClose.vue';
import IconSettings from '@/components/icons/IconSettings.vue';

defineProps<{
    title: string
}>()

const titleBar: Ref<HTMLElement | undefined> = ref();

onMounted(() => {
    if (!titleBar.value) {
        console.error('titlebar element not found')
        return;
    }

    titleBar.value.addEventListener('mousedown', () => window.saucer.start_drag());
});

function close() {
    window.saucer.call('close', []);
}
</script>

<template>
    <nav>
        <div class="title-bar">
            <div class="title" ref="titleBar">
                <span>{{ title }}</span>
            </div>
            <div class="controls">
                <button id="settings" @click="close">
                    <IconSettings></IconSettings>
                </button>
                <button id="close">
                    <IconClose></IconClose>
                </button>
            </div>
        </div>
    </nav>
</template>
<style scoped>
nav {
    position: fixed;
    z-index: 9999;
}

.title-bar {
    width: 100vw;
    background-color: rgba(0, 0, 0, 0.5);
    text-align: center;
    line-height: 48px;
    display: flex;
}

.title {
    width: 100%;
}

.controls {
    position: absolute;
    right: 0;
    top: 0;
    height: 100%;
    display: flex;
}

.controls button {
    margin: 0;
    display: block;
    background-color: rgba(0, 0, 0, 0.2);
    border: none;
    padding: 0;
    color: #fff;
    width: 48px;
    line-height: 48px;
    transition: 0.1s ease;
    font-size: 24px;
}

#close:hover {
    background-color: rgb(226, 46, 22);
}

#close:active {
    background-color: rgb(153, 33, 17);
}

#settings:hover {
    background-color: rgb(22, 151, 226);
}

#settings:active {
    background-color: rgb(18, 103, 153);
}
</style>

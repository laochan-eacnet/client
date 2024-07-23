<script setup lang="ts">
import { onMounted, ref } from 'vue';
const version = ref('r0 - unknown branch')

onMounted(async () => {
    version.value = await window.laochan.version();
});
</script>

<template>
    <div class="version">{{ version }}</div>
</template>

<style scoped>
.version {
    position: fixed;
    z-index: 30;
    left: 2em;
    bottom: 1em;
    opacity: 60%;
    font-family: 'Courier New', Courier, monospace;
}
</style>

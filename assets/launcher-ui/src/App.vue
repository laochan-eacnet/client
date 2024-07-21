<script setup lang="ts">
import { RouterView } from 'vue-router'
import TitleBar from './components/TitleBar.vue'
import Alert from './components/Alert.vue'
</script>

<template>
  <TitleBar title="Laochan Eacnet Launcher" />
  <RouterView v-slot="{ Component }">
    <transition name="blur-fade">
      <component :is="Component"></component>
    </transition>
  </RouterView>
  <Alert></Alert>
</template>

<style scoped>
.blur-fade-enter-active,
.blur-fade-leave-active {
  transition: 0.5s ease;
  position: absolute;
  top: 0;
  left: 0;
  height: 100vh;
  width: 100vw;
}

.blur-fade-enter-from {
  left: -10vw;
  opacity: 0;
}

.blur-fade-leave-to {
  left: 10vw;
  opacity: 0;
}

</style>

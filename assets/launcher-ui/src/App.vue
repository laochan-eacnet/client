<script setup lang="ts">
import { RouterView } from 'vue-router'
import TitleBar from './components/TitleBar.vue'
import Alert from './components/Alert.vue'
import Version from './components/Version.vue'
import Num from './components/Num.vue'
</script>

<template>
  <TitleBar title="Laochan Eacnet Launcher" />
  <div class="animated">
    <RouterView v-slot="{ Component }">
      <transition name="blur-fade">
        <component :is="Component"></component>
      </transition>
    </RouterView>
  </div>
  <Alert></Alert>
  <Version></Version>
  <Num></Num>
</template>

<style scoped>
.animated>* {
  transition: 0.5s ease;
  will-change: left, opacity;
}

.blur-fade-enter-active,
.blur-fade-leave-active {
  
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

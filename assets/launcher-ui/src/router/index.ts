import { createRouter, createWebHistory } from 'vue-router'
import HomeView from '../views/HomeView.vue'
import LauncherSettingView from '../views/LauncherSettingView.vue'

const router = createRouter({
  history: createWebHistory(import.meta.env.BASE_URL),  
  routes: [
    {
      path: '/',
      name: 'home',
      component: HomeView
    },
    {
      path: '/settings',
      name: 'launcher_settings',
      component: LauncherSettingView
    }
  ]
})

export default router

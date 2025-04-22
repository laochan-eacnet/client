<template>
    <main>
        <Transition :name="transMode">
            <div class="game-page ddr" v-if="game == 0">

            </div>
            <div class="game-page sdvx" v-else-if="game == 1">

            </div>
            <div class="game-page gitadora" v-else-if="game == 2">

            </div>
            <div class="game-page iidx" v-else-if="game == 3">

            </div>
        </Transition>
        <div class="game-select">
            <div id="text">
                <Transition :name="transMode">
                    <div v-if="game == 0">
                        DanceDanceRevolution GrandPrix
                    </div>
                    <div v-else-if="game == 1">
                        SOUND VOLTEX EXCEED GEAR
                    </div>
                    <div v-else-if="game == 2">
                        GITADORA
                    </div>
                    <div v-else-if="game == 3">
                        beatmania IIDX INFINITAS
                    </div>
                </Transition>
            </div>
        </div>
        <div class="game-icons">
            <div id="expander"></div>
            <div class="game ddr">
            </div>
            <div class="game sdvx">
            </div>
            <div class="game gitadora selected">
            </div>
            <div class="game iidx">
            </div>
        </div>
    </main>
</template>

<script setup lang="ts">
import { onMounted, ref } from 'vue';

const game = ref(2);
const transMode = ref('down');

onMounted(() => {
    const expander = document.querySelector<HTMLDivElement>('#expander');
    const imgs = document.querySelectorAll<HTMLDivElement>('.game-icons>.game');
    console.log(imgs);

    imgs.forEach((img, index) => {
        console.log(index)
        img.addEventListener('click', function () {
            if (game.value == index) {
                return;
            }

            const dist = (document.body.clientHeight / 2 - 64 + 150) - (index * 80);

            imgs.forEach(img => {
                img.classList.remove('selected');
            });

            this.classList.add('selected');

            expander!.style.height = dist + "px";

            if (index < game.value) {
                transMode.value = 'up'
            } else if (index > game.value) {
                transMode.value = 'down'
            }

            game.value = index;
        });
    });
});
</script>

<style lang="scss">
.up-enter-active,
.up-leave-active,
.up-enter-from,
.up-leave-to,
.down-enter-active,
.down-leave-active,
.down-enter-from,
.down-leave-to {
    transition: all 0.3s ease;
}

.up-enter-active,
.up-leave-active,
.down-enter-active,
.down-leave-active {
    position: absolute;
    top: 0;
}

.up-enter-from {
    opacity: 0;
    top: -32px;
}

.up-leave-to {
    opacity: 0;
    top: 32px;
}

.down-enter-from {
    opacity: 0;
    top: 32px;
}

.down-leave-to {
    opacity: 0;
    top: -32px;
}

main {
    height: 100vh;
    display: flex;
    align-items: stretch;
    background-color: #000;
}

.game-page {
    width: 100vw;
    height: 100vh;
    background-size: cover;
    background-position: center;
}

.game-page.sdvx {
    background-image: url(@/assets/sdvx.jpg);
}

.game-page.gitadora {
    background-image: url(@/assets/gitadora.jpg);
}

.game-page.ddr {
    background-image: url(@/assets/ddr_bg.jpg);
    opacity: 0.8;
    filter: blur(5px) brightness(0.65) !important;
}

.game-select {
    z-index: 1;
    background: linear-gradient(to top, rgba(0, 0, 0, 0.35) 0%, rgba(0, 0, 0, 0.5) 70%, rgba(20, 20, 20, 0.5) 71%, rgba(80, 80, 80, 0.5) 100%);
    border-top: 1px solid rgba(255, 255, 255, 0.6);
    border-bottom: 1px solid rgba(127, 127, 127, 0.1);
    position: absolute;
    left: 0px;
    top: calc(50vh - 36px + 150px);
    height: 72px;
    width: 100vw;
    box-shadow: 0px 5px 16px rgba(0, 0, 0, 0.5);
    backdrop-filter: blur(5px);
}

.game-select>#text {
    line-height: 72px;
    font-size: 32px;
    margin-left: 256px;
    text-shadow: 0px 2px 0px rgba(0, 0, 0, 0.1),
        0px 0px 5px rgba(255, 255, 255, 0.4),
        0px 0px 10px rgba(255, 255, 255, 0.5),
        0px 0px 20px rgba(255, 255, 255, 0.6);
}

.game-icons {
    z-index: 2;
    display: flex;
    flex-direction: column;
    justify-content: flex-start;
    width: 48px;
    height: 200vh;
    position: absolute;
    left: 72px;
}

.game.ddr {
    background-image: url(@/assets/icons/ddr.ico);
}

.game.sdvx {
    background-image: url(@/assets/icons/sdvx.ico);
}

.game.gitadora {
    background-image: url(@/assets/icons/gtdr.ico);
}


.game-icons>.game {
    cursor: pointer;
    margin: 16px;
    border: 1px solid rgba(255, 255, 255, 0.1);
    border-top: 1px solid #fff;
    border-radius: 5px;
    width: 48px;
    height: 48px;
    overflow: hidden;
    transition: all 0.2s ease;
    width: 48px;
    height: 48px;
    display: block;
    background-size: cover;
    position: relative;
    box-shadow: 0px 16px 16px rgba(0, 0, 0, 0.5), 0px 8px 8px rgba(0, 0, 0, 0.5), 0px 4px 4px rgba(0, 0, 0, 0.5);
}

.game-icons>.game::after {
    content: ' ';
    position: absolute;
    width: 100%;
    height: 100%;
    background: linear-gradient(to top, rgba(0, 0, 0, 0.1) 0%, rgba(0, 0, 0, 0.2) 70%, rgba(20, 20, 20, 0.1) 71%, rgba(80, 80, 80, 0) 100%);
}

.game-icons>.game.selected {
    width: 96px;
    height: 96px;
}


#expander {
    transition: height 0.2s ease;
    height: 226px;
}
</style>

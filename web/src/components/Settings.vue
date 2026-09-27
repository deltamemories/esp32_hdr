<script setup lang="ts">
import { ref } from 'vue'

const exposure = ref<number>(0)
const iso = ref<number>(0)

async function updateSettings() {
  try {
    await fetch('/api/settings', {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
      },
      body: JSON.stringify({
        exposure: exposure.value,
        iso: iso.value,
      }),
    })
  } catch (error) {
    console.error(error)
  }
}
</script>

<template>
  <div class="settings">
    <h2>Camera settings</h2>

    <div class="field">
      <label for="exposure (in lines)">Exposure: {{ exposure }}</label>
      <input
        id="exposure"
        v-model.number="exposure"
        type="range"
        min="0"
        max="1200"
        step="1"
        @change="updateSettings"
      />
    </div>

    <div class="field">
      <label for="shutter">ISO: {{ iso }}</label>
      <input
        id="iso"
        v-model.number="iso"
        type="range"
        min="0"
        max="127"
        step="1"
        @change="updateSettings"
      />
    </div>
  </div>
</template>

<style scoped>
.settings {
  display: flex;
  flex-direction: column;
  gap: 16px;
  max-width: 300px;
  padding: 16px;
  font-family: sans-serif;
}

.field {
  display: flex;
  flex-direction: column;
  gap: 8px;
}
</style>

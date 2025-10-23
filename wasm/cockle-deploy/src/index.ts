import { ShellManager } from '@jupyterlite/cockle';
import "./style/deployment.css"
import { Deployment } from "./deployment";

document.addEventListener("DOMContentLoaded", async () => {
  const baseUrl = window.location.href;
  const shellManager = new ShellManager();
  const browsingContextId = await shellManager.installServiceWorker(baseUrl);

  const targetDiv: HTMLElement = document.getElementById('targetdiv')!;
  const playground = new Deployment({ baseUrl, browsingContextId, shellManager, targetDiv });
  await playground.start();
})

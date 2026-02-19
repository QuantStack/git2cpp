import { ShellManager } from '@jupyterlite/cockle';
import "./style/deployment.css"
import { IDeployment } from './defs';
import { Deployment } from "./deployment";

document.addEventListener("DOMContentLoaded", async () => {
  const baseUrl = window.location.href;
  const shellManager = new ShellManager();
  const browsingContextId = await shellManager.installServiceWorker(baseUrl);
  const targetDiv: HTMLElement = document.getElementById('targetdiv')!;

  const options: IDeployment.IOptions = {
    baseUrl, browsingContextId, shellManager, targetDiv
  }

  // See if a local CORS proxy is available where we are expecting it by checking if it is there.
  // This isn't good practice but is OK for local manual testing.
  try {
    const corsProxy = "http://localhost:8881/"
    const response = await fetch(corsProxy);
    if (response.ok && response.type == "cors") {
      options.useLocalCors = corsProxy;
    }
  } catch (error) {}

  const playground = new Deployment(options);
  await playground.start();
})

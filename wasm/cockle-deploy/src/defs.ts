import { IShellManager } from '@jupyterlite/cockle';

export namespace IDeployment {
  export interface IOptions {
    baseUrl: string;
    browsingContextId: string;
    shellManager: IShellManager;
    targetDiv: HTMLElement;
  }
}

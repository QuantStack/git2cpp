import { Shell } from '@jupyterlite/cockle'
import { FitAddon } from '@xterm/addon-fit'
import { Terminal } from '@xterm/xterm'
import { IDeployment } from './defs'

export class Deployment {
  constructor(options: IDeployment.IOptions) {
    this._targetDiv = options.targetDiv;

    const termOptions = {
      rows: 50,
      theme: {
        foreground: "ivory",
        background: "#111111",
        cursor: "silver"
      },
    }
    this._term = new Terminal(termOptions)

    this._fitAddon = new FitAddon()
    this._term.loadAddon(this._fitAddon)

    const { baseUrl, browsingContextId, shellManager } = options;

    this._shell = new Shell({
      browsingContextId,
      baseUrl,
      wasmBaseUrl: baseUrl,
      shellManager,
      outputCallback: this.outputCallback.bind(this),
    })
  }

  async start(): Promise<void> {
    this._term!.onResize(async (arg: any) => await this.onResize(arg))
    this._term!.onData(async (data: string) => await this.onData(data))

    const resizeObserver = new ResizeObserver((entries) => {
      this._fitAddon!.fit()
    })

    this._term!.open(this._targetDiv)
    await this._shell.start()
    resizeObserver.observe(this._targetDiv)
  }

  async onData(data: string): Promise<void> {
    await this._shell.input(data)
  }

  async onResize(arg: any): Promise<void> {
    await this._shell.setSize(arg.rows, arg.cols)
  }

  private outputCallback(text: string): void {
    this._term!.write(text)
  }

  private _targetDiv: HTMLElement;
  private _term: Terminal
  private _fitAddon: FitAddon
  private _shell: Shell
}

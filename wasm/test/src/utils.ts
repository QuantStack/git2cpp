import { IOutputCallback } from '@jupyterlite/cockle';

/**
 * Provides outputCallback to mock a terminal.
 */
export class MockTerminalOutput {
  constructor() {}

  callback: IOutputCallback = (output: string) => {
    this._text = this._text + output.replace('\r\n', '\n');
  };

  clear() {
    this._text = '';
  }

  get text(): string {
    return this._text;
  }

  textAndClear(): string {
    const ret = this.text;
    this.clear();
    return ret;
  }

  private _text: string = '';
}

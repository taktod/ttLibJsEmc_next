import * as doe from "../emcts/decoder/opusDecoder.emc";
import * as dse from "../emcts/decoder/speexDecoder.emc";
import * as dte from "../emcts/decoder/theoraDecoder.emc";
import * as eoe from "../emcts/encoder/opusEncoder.emc";
import * as ese from "../emcts/encoder/speexEncoder.emc";
import * as ete from "../emcts/encoder/theoraEncoder.emc";
import * as rsr from "../emcts/resampler/speexdspResampler.emc";

export namespace decoder {
  export class OpusDecoder   extends doe.OpusDecoder{};
  export class SpeexDecoder  extends dse.SpeexDecoder{};
  export class TheoraDecoder extends dte.TheoraDecoder{};
}
export namespace encoder {
  export class OpusEncoder   extends eoe.OpusEncoder{};
  export class SpeexEncoder  extends ese.SpeexEncoder{};
  export class TheoraEncoder extends ete.TheoraEncoder{};
}
export namespace resampler {
  export class speexdspResampler extends rsr.SpeexdspResampler{};
}
export class OpusDecoder       extends doe.OpusDecoder{};
export class SpeexDecoder      extends dse.SpeexDecoder{};
export class TheoraDecoder     extends dte.TheoraDecoder{};
export class OpusEncoder       extends eoe.OpusEncoder{};
export class SpeexEncoder      extends ese.SpeexEncoder{};
export class TheoraEncoder     extends ete.TheoraEncoder{};
export class speexdspResampler extends rsr.SpeexdspResampler{};

# win/README-Windows.txt �̖�
gnuplot �o�[�W���� 5.4 -- Windows �p�o�C�i���z�t
=======================================================================

gnuplot �́A�R�}���h���͕����̑Θb�@�\�̃O���t���[�e�B���e�B�ŁALinux,
OSX, Windows, VMS, ���̑������̃v���b�g�z�[����œ��삵�܂��B���̃\�t�g
�E�F�A�ɂ͒��쌠������܂����A�t���[�ɔz�z����Ă��܂� (���Ȃ킿�A����
�ɑΉ����x�����K�v�͂���܂���)�B���́A�Ȋw�҂�w�������w�֐���f�[�^��
�ǂ����o�����邽�߂̃O���t�v���O�����ł����B

gnuplot �́A�Ȑ� (2 ����)�A����ыȖ� (3 ����) �̃O���t�̗����������ł�
�܂��B�Ȗʂ́A�w�肵���֐��ɍ����A3 �������W��ԏ��Y���Ԗڂ̌`���ŕ`
�悵����A�܂� x-y ���ʏ�̓������Ƃ��ĕ`�悵����ł��܂��B
2 �����`��ł́A�ܐ��O���t�A�_�O���t�A�_�O���t�A�������A�q�X�g�O�����A3
�����f�[�^�̓������ˉe�ȂǁA�����̕`��X�^�C�����T�|�[�g���Ă��܂��B�O
���t�ɂ́A�C�ӂ̃��x������A���̃��x���A�^�C�g���A�����A�O���t�̖}��
�Ȃǂ̃��x���Â����s���܂��B


�͂��߂�
--------

gnuplot �̐V�K���[�U�́Agnuplot �N����� `help` �ƃ^�C�v���邱�Ƃŕ\��
������ʓI�Ȑ���������ǂނ��Ƃ���n�߁A������ `plot` �R�}���h�Ɋւ�
����� (`help plot` �Ɠ��͂��Ă�������) ��ǂ�ł��������Bgnuplot �̃}
�j���A�� (�I�����C���w���v�ɍ����`���ŏ�����Ă��܂�) �́APDF �t�@�C��
�Ƃ��Ă��u����Ă��܂� (��: gnuplot �ɂ͓��{��� PDF �}�j���A���͕t��
���Ă��܂���B���{��� PDF �}�j���A���́A���{��}�j���A���T�C�g
  http://takeno.iee.niit.ac.jp/~foo/gp-jman/
���Q�Ƃ��Ă�������)�B

'demo' �f�B���N�g���ɂ́A��������̃e�X�g�p�A�T���v���p�̃X�N���v�g���u
���Ă���܂��B`test` �� `load "all.dem"` �����s���邩�A�܂��̓C���^�[�l
�b�g��̈ȉ��̃f�����Q�Ƃ��Ă��������B
  http://www.gnuplot.info/screenshots/index.html#demos


���C�Z���X
----------

�g�p���� (���쌠) �����ɂ��ẮACopyright �t�@�C�����Q�Ƃ��Ă��������B

gnuplot �Ɋ܂܂�� "GNU" �́AFree Software Foundation �Ƃ͊֌W�u�Ȃ��v�A
���܂��܈�v�����ɂ����܂��� (����ɂ͒����b������܂��B�ڍׂ� gnuplot
FAQ ���Q��)�B����āAgnuplot �� GPL (GNU Public License) copyleft �ŃJ
�o�[����Ă͂��炸�A���O�̒��쌠�����������Ă��āA����͂��ׂẴ\�[�X
�R�[�h�ɂ��y��ł��܂��B�������A�֘A����h���C�o�ƃT�|�[�g���[�e�B���e
�B�ɂ́A��d���C�Z���X�ɂȂ��Ă�����̂�����܂��B


gnuplot �o�C�i���z�z��
----------------------

* wgnuplot.exe:  GUI �ł̃f�t�H���g�� gnuplot ���s�t�@�C���B

* gnuplot.exe:  �e�L�X�g (�R���\�[��) ���[�h�ł� gnuplot ���s�t�@�C����
  ���̃v���b�g�z�[���̂��̂Ɠ��l�̊��S�ȃp�C�v�@�\�������܂��B
  wgnuplot.exe �Ɣ�r����ƁA���̃v���O�����͕W�����͂���̃R�}���h����
  ���t���A�o�̓��b�Z�[�W��W���o�͂ɂ��o���܂��B����͈ȑO�� gnuplot ��
  �g���Ă��� pgnuplot.exe �ɑ�����̂ŁA�Ⴆ�� Octave 
  (www.octave.org) �̂悤�ȑ��̃A�v���P�[�V�����̃O���t�`��G���W���Ƃ�
  �Ďg�p���邱�Ƃ��ł��܂��B

* �����^�C�����C�u�����t�@�C��
  gnuplot ���K�v�Ƃ��郉���^�C�����C�u�����t�@�C�� (libfreetype-6.dll 
  ��) �����̃p�b�P�[�W�Ɋ܂܂�Ă��܂��B�e�����^�C�����C�u�����̃��C�Z
  ���X�t�@�C���� 'license' �f�B���N�g�����ɒu����Ă��܂��B


�Θb�^�o�͌`��
--------------

Windows �p�� gnuplot �́A3 ��ނ̑Θb�^�o�͌`�� windows, wxt, qt ���
���Ă��܂��B��� 2 �́A���̃v���b�g�t�H�[���ł����p�\�ł��B������
��������A�A���`�G�C���A�X�A�q���e�B���O���Ƃ��Ȃ��I�[�o�[�T���v�����O
�𗘗p�������i���o�͂𐶐����Agnuplot �̍ŋ߂̋@�\���ׂĂ��T�|�[�g����
���܂����A�O���t�@�C����N���b�v�{�[�h�ɏo�͂ł���`����A����̃T�|�[
�g�Apersist ���[�h�̋����ɂ��Ă͈Ⴂ������܂��Bwxt �́Apngcairo �� 
pdfcairo �o�͌`���Ɠ����`�惋�[�`�����g�p���Ă���̂ŁA�O���t���Θb
�^�ɕۑ����邱�Ƃ��\�ł��Bwindows �o�͌`���̃O���t�E�B���h�E�́A
wgnuplot �̃e�L�X�g�E�B���h�E�Ƀh�b�L���O�����邱�Ƃ��\�ł��B

�f�t�H���g�ł́AWindows �p�� gnuplot �� wxt �o�͌`�����g�p���܂����A��
��́AGNUTERM ���ϐ���ݒ肷�邱�ƂŕύX�ł��܂��B���́u���ϐ��v��
�߂��Q�Ƃ��Ă��������B�܂��́A���Ȃ��� gnuplot.ini ��
    set term windows / wxt / qt
�Ƃ����s��ǉ�����Ƃ������@������܂��B`help startup` ���Q�Ƃ��Ă�����
���B


�C���X�g�[��
------------

gnuplot �̓C���X�g�[�����̌`���ɂȂ��Ă��āA����͊�{�I�ɂ́A���Ȃ�
���I������I�v�V�����ɉ����āA�ȉ��̂悤�ɓ��삵�܂�:

* �Ⴆ�� C:\Program Files\gnuplot ���̂��Ȃ����I�������f�B���N�g������
  ���̃p�b�P�[�W (�܂��͂��̈ꕔ��) ��W�J���܂��B

* �f�X�N�g�b�v��Ɓu�N�C�b�N�N���v�̏ꏊ (Windows XP, Vista �̏ꍇ) ��
  wgnuplot �̃V���[�g�J�b�g�A�C�R�����쐬���܂��B����ɁA�X�^�[�g�A�b�v
  ���j���[���ɂ��̃v���O������w���v�A�t�������Agnuplot �̃C���^�[�l�b
  �g�T�C�g�A�f���X�N���v�g�ւ̃����N��ǉ����܂��B
  (��: �u�N�C�b�N�N���v�́u�X�^�[�g�v���E�N���b�N���āu�v���p�e�B�v�A
  �u�^�X�N�o�[�v�Ɛi��Łu�N�C�b�N�N����\������v�Ƀ`�F�b�N���邱�Ƃ�
  �\������܂��B)

* *.gp �� *.gpl, *.plt �̊g���q�̃t�@�C���� wgnuplot ���J���悤�֘A�t��
  �����܂��BWindows 7 �ȍ~�Ŋ֘A�t����ύX����ɂ́A[�R���g���[���p�l��]�A
  [�v���O����]�A[����̃v���O����]�A[�֘A�t����ݒ肷��]�A�Ɛi��ł���
  �����B�����Ĉꗗ�̒�����t�@�C���̌`����I�����A[�v���O�����̕ύX] ��
  �N���b�N���Ă��������B

* gnuplot ���s�t�@�C���ւ̃p�X�� PATH ���ϐ��ɒǉ����܂��B����ɂ��
  �R�}���h���C���� `gnuplot' �� `wgnuplot' �Ɠ��͂���� gnuplot ���N��
  �ł���悤�ɂȂ�܂��B

* Windows �̃G�N�X�v���[���[�� "�t�@�C�������w�肵�Ď��s" �̃_�C�A���O
  �E�B���h�E�̃V���[�g�J�b�g�� gnuplot ��ǉ����܂��B����ŁA�P�� 
  Windows �L�[�������� `wgnuplot' ��I�����Ď��s���邱�Ƃ� wgnuplot ��
  �N���ł���悤�ɂȂ�܂��B

* �f�t�H���g�̏o�͌`�� (terminal) �� wxt/windows/qt ����I������ƁA�C
  ���X�g�[���͂���ɉ����� GNUTERM ���ϐ����X�V���܂��B

* �f���X�N���v�g���C���X�g�[�������ꍇ�́A�������܂ރf�B���N�g����
  GNUPLOT_LIB �����p�X�Ɋ܂܂�܂��B�ȉ��Q�ƁB

�J�X�^�}�C�Y:
gnuplot �̓��[�U�̃A�v���P�[�V�����f�[�^�f�B���N�g�� %APPDATA% ������
��΁A�N�����ɂ����ɂ��� gnuplot.ini ���܂����s���܂��Bwgnuplot �̃e�L
�X�g�E�B���h�E�� windows �o�͌`���́A�A�v���P�[�V�����f�[�^�f�B���N�g��
���� wgnuplot.ini ����ݒ��ǂݍ��݁A���̃t�@�C���ɐݒ��ۑ����܂��B
`help wgnuplot.ini` ���Q�Ƃ��Ă��������B


�t�H���g
--------

�O���t�B�J���e�L�X�g�E�B���h�E (wgnuplot.exe):
  �Θb�E�B���h�E�̃t�H���g�́A�E�B���h�E�㕔�́u�I�v�V�����v�̃A�C�R��
  ���A�܂��̓E�B���h�E�ŏ㕔���E�N���b�N���ďo�郁�j���[���� "Choose 
  Font..." ��I�����ĕύX�ł��܂��B�Â� "Terminal" (gnuplot 4.4 �ł̃f
  �t�H���g) �̂悤�ȃt�H���g�ł͂Ȃ��A"Consolas" �̂悤�ȐV���� 
  Truetype �t�H���g���g�p���邱�Ƃ������������܂� (��: ���{����g�p��
  ��ꍇ�́A�uMS �S�V�b�N�v��uMS �����v�Ȃǂ̓��{��̃t�H���g���w�肷
  ��Ƃ����ł��傤)�B���̕ύX��ێ����邽�߂ɁA�������j���[�ɂ��� 
  "Update wgnuplot.ini" �����s���Ă��������B

�R���\�[���E�B���h�E (gnuplot.exe):
  �g���������������\������Ȃ���΁A"Consolas" �� "Lucida Console" �Ȃ�
  �̔񃉃X�^���C�Y�`���̃R���\�[���t�H���g�ɕύX����K�v������܂��B��
  ��̓R���\�[���E�B���h�E�́u�v���p�e�B�v�ōs���܂��B


�G���R�[�f�B���O
----------------

Windows ��Agnuplot �̃o�[�W���� 5.2 �ȍ~�ł́Agnuplot ���T�|�[�g���邷
�ׂẴG���R�[�f�B���O (UTF-8 ���܂ށB�ȉ��Q��: `help encoding`) ���g��
�ăR�}���h���C�����͂��s�����Ƃ��ł��܂��B�f�t�H���g�ł́Agnuplot �́A
�T�|�[�g���Ă���΁A���̃V�X�e���� ANSI �R�[�h�y�[�W�Ɉ�v����G���R�[
�f�B���O���g���܂����A���Ȃ��� gnuplot.ini �t�@�C�� (���Ő���) �ɁA
`set encoding utf8` �Ƃ����s��ǉ����邱�Ƃ𐄏����܂��B�R�}���h���C��
�̃��j�R�[�h���͂� BMP (��{�������) �ɐ�������Ă��܂����A�X�N���v�g
�ł� (�ȑO�̃o�[�W�����Ɠ��l) ���ׂĂ̕������g���܂��B


�n��Ή�
--------

gnuplot �́A���j���[�ƃw���v�t�@�C���̒n��Ή����T�|�[�g���Ă��āA�f�t
�H���g�ł� gnuplot �� wgnuplot-XX.mnu �� wgnuplot-XX.chm ��ǂݍ�������
���܂��B�����ŁAXX �� 2 �����̌���R�[�h�ł��B���݂̂Ƃ���A�p�� (�f�t
�H���g) �Ɠ��{�� (ja) ���T�|�[�g���Ă��܂����A���̒n��Ή��t�@�C������
�W���Ă��܂��B

�����I�ɓ���̌�����g���悤�ɂ���ɂ́Awgnuplot.ini �t�@�C����
  Language=XX
�����Ă��������B���̃t�@�C���́A���Ȃ��� %APPDATA% �f�B���N�g���ɒu��
��Ă��܂��B�Ⴆ�Ήp��̃��j���[�œ��{��̃w���v���������A�Ƃ���������
�����ݒ肪��������΁Awgnuplot.ini �Ɉȉ��̍s��ǉ�����΂ł��܂�:
  HelpFile=wgnuplot-ja.chm
  MenuFile=wgnuplot.mnu

���݂� gnuplot �����猾����̐ݒ��ύX������@�͂Ȃ����Ƃɒ��ӂ��Ă�
�������B


���ϐ�
--------

�T�|�[�g������ϐ��̈ꗗ������ɂ́Agnuplot ��� 'help environment'
�Ƃ��Ă��������B

���ϐ���ݒ�/�ύX����ɂ́AWindows NT/2000/XP/Vista �ł� [�R���g���[
���p�l��]�A[�V�X�e��]�A([�ڍאݒ�])�A[���ϐ�] �ŁAWIndows 7 �ł̓f�X
�N�g�b�v�̃R���s���[�^�A�C�R�����E�N���b�N���� [�v���p�e�B] ��I�����A
[�V�X�e��]�A[�V�X�e���̏ڍאݒ�]�A[�ڍאݒ�]�A[���ϐ�] �ōs���܂��B

* GNUTERM ����`����Ă���ƁA���̒l�̖��O�̏o�͌`�� (terminal) ���g�p
  ���܂��B����́Agnuplot �N�����ɔF�����ꂽ�����Ȃ�o�͌`�������D��
  ���܂����A�������t�@�C�� gnuplot.init �ŏ㏑���ł� (`help startup` �Q
  ��)�A�����Ă�����񂻂̌�̖����I�ȏo�͌`���̎w��ŕύX�ł��܂��B

* ���ϐ� GNUPLOT_LIB �́A�f�[�^��R�}���h�X�N���v�g�̒ǉ������f�B���N
  �g�����`����̂Ɏg���܂��B���̕ϐ��̒l�́A�P��̃f�B���N�g�����ł��A
  ��؂蕶�� ';' �ŋ�؂�ꂽ�����̃f�B���N�g�����X�g�ł��\���܂���B
  GNUPLOT_LIB �̓��e�́A`loadpath` �����ϐ��ɒǉ�����܂����A`save` ��
  `save set` �R�}���h�ŕۑ����܂���B�ڍׂ� 'help loadpath' ���Q�Ƃ���
  ���������B


�m���Ă���o�O
----------------

�V�����o�O�̏��ɂ��ẮA�o�O�ǐՃV�X�e��

    http://sourceforge.net/p/gnuplot/bugs/

���Q�Ƃ��Ă��������B

--------------------------------------------------------------------------------

The gnuplot team, 2020 4 ��

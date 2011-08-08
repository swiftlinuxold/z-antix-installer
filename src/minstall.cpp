//
//  Copyright (C) 2003-2010 by Warren Woodford
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//   See the License for the specific language governing permissions and
//   limitations under the License.
//

#include <QHeaderView>
#include "minstall.h"
#include "mmain.h"

MInstall::MInstall(QWidget *parent) : QWidget(parent) {
  setupUi(this);
  char line[260];
  char *tok;
  FILE *fp;
  int i;

  // timezone
  timezoneCombo->clear();
  fp = popen("awk -F '\\t' '!/^#/ { print $3 }' /usr/share/zoneinfo/zone.tab | sort", "r");
  if (fp != NULL) {
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
     if (line != NULL && strlen(line) > 1) {
        timezoneCombo->addItem(line);
      }
    }
    pclose(fp);
  }
  timezoneCombo->setCurrentIndex(timezoneCombo->findText("America/New York"));

  // keyboard
  system("ls -1 /usr/share/keymaps/i386/azerty > /tmp/mlocale");
  system("ls -1 /usr/share/keymaps/i386/qwerty >> /tmp/mlocale");
  system("ls -1 /usr/share/keymaps/i386/qwertz >> /tmp/mlocale");
  system("ls -1 /usr/share/keymaps/i386/dvorak >> /tmp/mlocale");
  system("ls -1 /usr/share/keymaps/i386/fgGIod >> /tmp/mlocale");
  keyboardCombo->clear();
  fp = popen("sort /tmp/mlocale", "r");
  if (fp != NULL) {
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line) - 9;
      line[i] = '\0';
     if (line != NULL && strlen(line) > 1) {
        keyboardCombo->addItem(line);
      }
    }
    pclose(fp);
  }
  keyboardCombo->setCurrentIndex(keyboardCombo->findText("us"));

  // locale
  localeCombo->clear();
  fp = popen("/usr/bin/locale -a", "r");
  if (fp != NULL) {
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
      tok = strtok(line, " ");
     if (tok != NULL && strlen(tok) > 1 && strncmp(tok, "#", 1) != 0) {
        localeCombo->addItem(tok);
      }
    }
    pclose(fp);
  }
  localeCombo->setCurrentIndex(localeCombo->findText("en_US"));

  proc = new QProcess(this);
  timer = new QTimer(this);

  // if an apple...
  if (system("hal-get-property --udi /org/freedesktop/Hal/devices/computer --key system.firmware.vendor | grep 'Apple'") == 0) {
//    rootTypeLabel->setEnabled(false);
    rootTypeCombo->removeItem(2);
    rearrangediskBox->setEnabled(false);
    installationTypeBox->setEnabled(false);
    grubCheckBox->setChecked(false);
    grubRootButton->setChecked(true);
    grubMbrButton->setEnabled(false);
//    swapCombo->setEnabled(false);
//    homeCombo->setEnabled(false);
    gmtCheckBox->setChecked(true);
  }

  //setup csView
  csView->header()->setMinimumSectionSize(150);
  csView->header()->resizeSection(0,150);
  QTreeWidgetItem *networkItem = new QTreeWidgetItem(csView);
  networkItem->setText(0, tr("Networking"));
//  networkItem->setSizeHint(0, QSize(180,10));
//  networkItem->setSizeHint(1, QSize(200,10));
  QString val = getCmdValue("dpkg -s guarddog | grep '^Status'", "ok", " ", " ");
  if (val.compare("installed") == 0) {
    // guarddog installed
    guarddogItem = new QTreeWidgetItem(networkItem);
    guarddogItem->setText(0, "guarddog");
    guarddogItem->setText(1, tr("Desktop firewall"));
    guarddogItem->setCheckState(0, Qt::Checked);
  } else {
    guarddogItem = NULL;
  }
  val = getCmdValue("dpkg -s wicd | grep '^Status'", "ok", " ", " ");
  if (val.compare("installed") == 0) {
  wicdItem = new QTreeWidgetItem(networkItem);
  wicdItem->setText(0, "wicd");
  wicdItem->setText(1, tr("Network connection"));
  wicdItem->setCheckState(0, Qt::Checked);
  } else {
    wicdItem = NULL;
  }
  
  networkItem->setExpanded(true);
  
  QTreeWidgetItem *hardwareItem = new QTreeWidgetItem(csView);
  hardwareItem->setText(0, tr("Hardware"));
  val = getCmdValue("dpkg -s cpufrequtils | grep '^Status'", "ok", " ", " ");
  if (val.compare("installed") == 0) {
    cpufreqItem = new QTreeWidgetItem(hardwareItem);
    cpufreqItem->setText(0, "cpufrequtils");
    cpufreqItem->setText(1, tr("CPU frequency, irqbalance"));
    cpufreqItem->setCheckState(0, Qt::Checked);
  } else {
    cpufreqItem = NULL;
  }
  
  val = getCmdValue("dpkg -s laptop-mode-tools | grep '^Status'", "ok", " ", " ");
  if (val.compare("installed") == 0) {
    laptopItem = new QTreeWidgetItem(hardwareItem);
    laptopItem->setText(0, "laptopmode");
    laptopItem->setText(1, tr("Laptop mode"));
    laptopItem->setCheckState(0, Qt::Checked);
  } else {
    laptopItem = NULL;
  }

  hardwareItem->setExpanded(true);

  QTreeWidgetItem *printItem = new QTreeWidgetItem(csView);
  printItem->setText(0, tr("Printing"));

  val = getCmdValue("dpkg -s cups | grep '^Status'", "ok", " ", " ");
  if (val.compare("installed") == 0) {
    cupsItem = new QTreeWidgetItem(printItem);
    cupsItem->setText(0, "cups");
    cupsItem->setText(1, tr("Linux and OSX printer service"));
    cupsItem->setCheckState(0, Qt::Checked);
    cupsItem->setExpanded(true);
    printItem->setExpanded(true);
  } else {
    cupsItem = NULL;
  }

  val = getCmdValue("dpkg -s samba | grep '^Status'", "ok", " ", " ");
  if (val.compare("installed") == 0) {
    sambaCheckBox->setChecked(true);
  } else {
    sambaCheckBox->setChecked(false);
    sambaCheckBox->setEnabled(false);
    computerGroupLabel->setEnabled(false);
    computerGroupEdit->setEnabled(false);
    computerGroupEdit->setText("");
  }

}

MInstall::~MInstall() {
}

/////////////////////////////////////////////////////////////////////////
// util functions

QString MInstall::getCmdOut(QString cmd) {
  char line[260];
  const char* ret = "";
  FILE* fp = popen(cmd.toAscii(), "r");
  if (fp == NULL) {
    return QString (ret);
  }
  int i;
  if (fgets(line, sizeof line, fp) != NULL) {
    i = strlen(line);
    line[--i] = '\0';
    ret = line;
  }
  pclose(fp);
  return QString (ret);
}

QStringList MInstall::getCmdOuts(QString cmd) {
  char line[260];
  FILE* fp = popen(cmd.toAscii(), "r");
  QStringList results;
  if (fp == NULL) {
    return results;
  }
  int i;
  while (fgets(line, sizeof line, fp) != NULL) {
    i = strlen(line);
    line[--i] = '\0';
    results.append(line);
  }
  pclose(fp);
  return results;
}

QString MInstall::getCmdValue(QString cmd, QString key, QString keydel, QString valdel) {
  const char *ret = "";
  char line[260];

  QStringList strings = getCmdOuts(cmd);
  for (QStringList::Iterator it = strings.begin(); it != strings.end(); ++it) {
    strcpy(line, ((QString)*it).toAscii());
    char* keyptr = strstr(line, key.toAscii());
    if (keyptr != NULL) {
      // key found
      strtok(keyptr, keydel.toAscii());
      const char* val = strtok(NULL, valdel.toAscii());
      if (val != NULL) {
        ret = val;
      }
      break;
    }
  }
  return QString (ret);
}

QStringList MInstall::getCmdValues(QString cmd, QString key, QString keydel, QString valdel) {
  char line[130];
  FILE* fp = popen(cmd.toAscii(), "r");
  QStringList results;
  if (fp == NULL) {
    return results;
  }
  int i;
  while (fgets(line, sizeof line, fp) != NULL) {
    i = strlen(line);
    line[--i] = '\0';
    char* keyptr = strstr(line, key.toAscii());
    if (keyptr != NULL) {
      // key found
      strtok(keyptr, keydel.toAscii());
      const char* val = strtok(NULL, valdel.toAscii());
      if (val != NULL) {
        results.append(val);
      }
    }
  }
  pclose(fp);
  return results;
}

bool MInstall::replaceStringInFile(QString oldtext, QString newtext, QString filepath) {

QString cmd = QString("sed -i 's/%1/%2/g' %3").arg(oldtext).arg(newtext).arg(filepath);
  if (system(cmd.toAscii()) != 0) {
    return false;
  }
  return true;
}

void MInstall::updateStatus(QString msg, int val) {
  installLabel->setText(msg.toAscii());
  progressBar->setValue(val);
  qApp->processEvents();
}

bool MInstall::mountPartition(QString dev, const char *point) {

  mkdir(point, 0755);
  QString cmd = QString("/bin/mount %1 %2").arg(dev).arg(point);
  if (system(cmd.toAscii()) != 0) {
    return false;
  }
  return true;
}

/////////////////////////////////////////////////////////////////////////
// install functions

bool isRootFormatted;
bool isHomeFormatted;
bool isFormatExt3;
bool isFormatReiserfs;

int MInstall::getPartitionNumber()
{
    return getCmdOut("cat /proc/partitions | grep '[h,s,v].[a-z][1-9]$' | wc -l").toInt();
}

// unmount antiX in case we are retrying
void MInstall::prepareToInstall() {

  updateStatus(tr("Ready to install Swift Linux filesystem"), 0);
  // unmount /home if it exists
  system("/bin/umount -l /mnt/antiX/home >/dev/null 2>&1");
  system("/bin/umount -l /mnt/antiX >/dev/null 2>&1");

  isRootFormatted = false;
  isHomeFormatted = false;
  isFormatExt3 = true;
}

bool MInstall::makeSwapPartition(QString dev) {
  QString cmd = QString("/sbin/mkswap %1").arg(dev);
  if (system(cmd.toAscii()) != 0) {
    // error
    return false;
  }
  return true;
}

bool MInstall::makeLinuxPartition(QString dev, const char *type, bool bad) {
  QString cmd;
  if (strncmp(type, "reis", 4) == 0) {
    cmd = QString("/sbin/mkreiserfs -q %1").arg(dev);
    if (system(cmd.toAscii()) != 0) {
      // error
      return false;
    }
  } else {
    if (strncmp(type, "ext4", 4) == 0) {
      // ext4
      if (bad) {
        // do with badblocks
        cmd = QString("/sbin/mkfs.ext4 -c %1").arg(dev);
       } else {
        // do no badblocks
        cmd = QString("/sbin/mkfs.ext4 %1").arg(dev);
      } 
    } else {
      // assume ext3
      if (bad) {
        // do with badblocks
        cmd = QString("/sbin/mkfs.ext3 -c %1").arg(dev);
       } else {
        // do no badblocks
        cmd = QString("/sbin/mkfs.ext3 %1").arg(dev);
      } 
    }    
    if (system(cmd.toAscii()) != 0) {
      // error
      return false;
    }
    system("sleep 1");

    if (strncmp(type, "ext4", 4) == 0) {
      // ext4 tuning
      cmd = QString("/sbin/tune2fs -c0 -C0 -i1m %1").arg(dev);
    } else {
      // ext3 tuning
      cmd = QString("/sbin/tune2fs -c0 -C0 -i1m %1").arg(dev);
    }
    if (system(cmd.toAscii()) != 0) {
      // error
    }
  }
  return true;
}

///////////////////////////////////////////////////////////////////////////
// in this case use all of the drive

bool MInstall::makeDefaultPartitions() {
  char line[130];
  int ans;

  QString drv = QString("/dev/%1").arg(diskCombo->currentText());
  QString rootdev = QString(drv).append("1");
  QString swapdev = QString(drv).append("2");
//  QString homedev = QString(drv).append("3");
  QString msg = QString(tr("Ok to format and use the entire disk (%1) for Swift Linux?")).arg(drv);
  ans = QMessageBox::information(0, QString::null, msg,
         tr("Yes"), tr("No"));
 if (ans != 0) {
    // don't format--stop install
    return false;
  }
  isRootFormatted = true;
  isHomeFormatted = false;
  isFormatExt3 = true;

  // entire disk, create partitions
  updateStatus(tr("Creating required partitions"), 1);

  // ready to partition
  // try to be sure that entire drive is available
  system("/sbin/swapoff -a 2>&1");

  // unmount /home part if it exists
//  QString cmd = QString("/bin/umount -l %1 >/dev/null 2>&1").arg(homedev);
//  if (system(cmd.toAscii()) != 0) {
    // error
//  }
  // unmount root part
  QString cmd = QString("/bin/umount -l %1 >/dev/null 2>&1").arg(rootdev);
  if (system(cmd.toAscii()) != 0) {
    // error
  }

  bool free = false;
  bool fiok = true;
  int fi = freeSpaceEdit->text().toInt(&fiok,10);
  if (!fiok) {
    fi = 0;
  }

  const char *tstr;                    // total size

  // calculate new partition sizes
  // get the total disk size
  sleep(1);
  cmd = QString("/sbin/sfdisk -s %1").arg(drv);
  FILE *fp = popen(cmd.toAscii(), "r");
  fgets(line, sizeof line, fp);
  tstr = strtok(line," ");
  pclose(fp);
  int sz = atoi(tstr);
  sz = sz / 1024;
  // pre-compensate for rounding errors in disk geometry
  sz = sz - 32;
  // 2048 swap should be ample
  int si = 2048;
  if (sz < 2048) {
    si = 128;
  } else if (sz < 3096) {
    si = 256;
  } else if (sz < 4096) {
    si = 512;
  } else if (sz < 12288) {
    si = 1024;
  }
  int ri = sz - si;

  if (fi > 0 && ri > 8192) {
      // allow fi
      // ri is capped until fi is satisfied
      if (fi > ri - 8192) {
        fi = ri - 8192;
      }
      ri = ri - fi;
      free = true;             // free space will be unallocated
  } else {
    // no fi
    fi = 0;
    free = false;		// no free space
  }

  // new partition table
  cmd = QString("/bin/dd if=/dev/zero of=%1 bs=512 count=100").arg(drv);
  system(cmd.toAscii());
  cmd = QString("/sbin/sfdisk -uM %1").arg(drv);
  fp = popen(cmd.toAscii(), "w");
  if (fp != NULL) {
    if (free) {
      cmd = QString(",%1,L,*\n,%2,S\n,,\n;\ny\n").arg(ri).arg(si);
    } else {
      cmd = QString(",%1,L,*\n,,S\n,,\n;\ny\n").arg(ri);
    }
    fputs(cmd.toAscii(), fp);
    fputc(EOF, fp);
    pclose(fp);
  } else {
    // error
    return false;
  }

  updateStatus(tr("Formatting swap partition"), 2);
  system("sleep 1");
  if (!makeSwapPartition(swapdev)) {
    return false;
  }
  system("sleep 1");
  system("/usr/sbin/buildfstab -r");
  system("/sbin/swapon -a 2>&1");

  updateStatus(tr("Formatting root partition"), 3);
  if (!makeLinuxPartition(rootdev, "ext3", false)) {
    return false;
  }

  system("sleep 1");
  // mount partitions
  if (!mountPartition(rootdev, "/mnt/antiX")) {
    return false;
  }

  // on root, make sure it exists
  system("sleep 1");
  mkdir("/mnt/antiX/home",0755);

  on_diskCombo_activated();
  rootCombo->setCurrentIndex(0);
  on_rootCombo_activated();
  swapCombo->setCurrentIndex(1);
  on_swapCombo_activated();
  homeCombo->setCurrentIndex(0);

  return true;
}

///////////////////////////////////////////////////////////////////////////
// Make the chosen partitions and mount them

bool MInstall::makeChosenPartitions() {
  char line[130];
  QString msg;
  int ans;
  char type[20];
  QString cmd;

  QString drv = QString("/dev/%1").arg(diskCombo->currentText());

  // get config
  strncpy(type, rootTypeCombo->currentText().toAscii(), 4);

  strcpy(line, rootCombo->currentText().toAscii());
  char *tok = strtok(line, " -");
  QString rootdev = QString("/dev/%1").arg(tok);

  strcpy(line, swapCombo->currentText().toAscii());
  tok = strtok(line, " -");
  QString swapdev = QString("/dev/%1").arg(tok);

  strcpy(line, homeCombo->currentText().toAscii());
  tok = strtok(line, " -");
  QString homedev = QString("/dev/%1").arg(tok);

  if (rootdev.compare("/dev/none") == 0) {
    QMessageBox::critical(0, QString::null,
      tr("You must choose a root partition.\nThe root partition must be at 1.5 GB."));
    return false;
  }
  if (!rootCombo->currentText().contains("linux", Qt::CaseInsensitive)) {
    msg = QString(tr("The partition you selected for root, appears to be a MS-Windows partition.  Are you sure you want to reformat this partition?")).arg(rootdev);
    ans = QMessageBox::warning(0, QString::null, msg,
        tr("Yes"), tr("No"));
    if (ans != 0) {
      // don't format--stop install
      return false;
    }
  }
  if (!(saveHomeCheck->isChecked() && homedev.compare("/dev/root") == 0)) {
    msg = QString(tr("Ok to format and destroy all data on \n%1 for the / (root) partition?")).arg(rootdev);
  } else {
    msg = QString(tr("Ok to use (no format) \n%1 as the / (root) partition?")).arg(rootdev);
  }
  ans = QMessageBox::warning(0, QString::null, msg,
        tr("Yes"), tr("No"));
  if (ans != 0) {
    // don't format--stop install
    return false;
  }

  // format swap
  if (swapdev.compare("/dev/none") != 0) {
    msg = QString(tr("Ok to format and destroy all data on \n%1 for the swap partition?")).arg(swapdev);
    ans = QMessageBox::warning(0, QString::null, msg,
        tr("Yes"), tr("No"));
  if (ans != 0) {
      // don't format--stop install
      return false;
    }
  }
  
  // format /home?
  if (homedev.compare("/dev/root") != 0) {
    if (!rootCombo->currentText().contains("linux", Qt::CaseInsensitive)) {
      msg = QString(tr("The partition you selected for /home, appears to be a MS-Windows partition.  Are you sure you want to reformat this partition?")).arg(rootdev);
      ans = QMessageBox::warning(0, QString::null, msg,
        tr("Yes"), tr("No"));
      if (ans != 0) {
        // don't format--stop install
        return false;
      }
    }
    if (saveHomeCheck->isChecked()) {
      msg = QString(tr("Ok to reuse (no reformat) %1 as the /home partition?")).arg(homedev);
    } else {
      msg = QString(tr("Ok to format and destroy all data on %1 for the /home partition?")).arg(homedev);
    }

    ans = QMessageBox::warning(0, QString::null, msg,
        tr("Yes"), tr("No"));
  if (ans != 0) {
      // don't format--stop install
      return false;
    }
  }

  updateStatus(tr("Preparing required partitions"), 1);

  // unmount /home part if it exists
  if (homedev.compare("/dev/root") != 0) {
    // has homedev
    cmd = QString("pumount %1").arg(homedev);
    if (system(cmd.toAscii()) != 0) {
      // error
      if (swapoff(homedev.toAscii()) != 0) {
      }
    }
  }

  // unmount root part
  cmd = QString("pumount %1").arg(rootdev);
  if (system(cmd.toAscii()) != 0) {
    // error
    if (swapoff(rootdev.toAscii()) != 0) {
    }
  }

  if (swapdev.compare("/dev/none") != 0) {
    if (swapoff(swapdev.toAscii()) != 0) {
      cmd = QString("pumount %1").arg(swapdev);
      if (system(cmd.toAscii()) != 0) {
      }
    }
    updateStatus(tr("Formatting swap partition"), 2);
    // always set type
    QString cmd = QString("/sbin/sfdisk %1 -c %2 82").arg(swapdev.mid(0,8)).arg(swapdev.mid(8));
    system(cmd.toAscii());
    system("sleep 1");
    if (!makeSwapPartition(swapdev)) {
      return false;
    }
    // enable the new swap partition asap
    system("sleep 1");
    swapon(swapdev.toAscii(),0);
  }

  // maybe format root
  if (!(saveHomeCheck->isChecked() && homedev.compare("/dev/root") == 0)) {
    updateStatus(tr("Formatting the / (root) partition"), 3);
    // always set type
    QString cmd = QString("/sbin/sfdisk %1 -c %2 83").arg(rootdev.mid(0,8)).arg(rootdev.mid(8));
    system(cmd.toAscii());
    system("sleep 1");
    if (!makeLinuxPartition(rootdev, type, badblocksCheck->isChecked())) {
      return false;
    }
    system("sleep 1");
    if (!mountPartition(rootdev, "/mnt/antiX")) {
      return false;
    }
    isRootFormatted = true;
    if (strncmp(type, "ext4", 4) == 0) {
      isFormatExt3 = false;
      isFormatReiserfs = false;
    } else if (strncmp(type, "reis", 4) == 0) {
      isFormatExt3 = false;
      isFormatReiserfs = true;
    } else {
      isFormatExt3 = true;
      isFormatReiserfs = false;
    }
  }
  // maybe format home
  if (saveHomeCheck->isChecked()) {
    // save home
    if (homedev.compare("/dev/root") != 0) {
      // not on root
      system("rm -r -d /mnt/antiX/home >/dev/null 2>&1");
      updateStatus(tr("Mounting the /home partition"), 8);
      if (!mountPartition(homedev, "/mnt/antiX/home")) {
        return false;
      }
    } else {
      // on root, make sure it exists
      system("sleep 1");
      mkdir("/mnt/antiX/home",0755);
    }
  } else {
    // don't save home
    system("/bin/rm -r -d /mnt/antiX/home >/dev/null 2>&1");
    mkdir("/mnt/antiX/home",0755);
    if (homedev.compare("/dev/root") != 0) {
      // not on root
      updateStatus(tr("Formatting the /home partition"), 8);
      // always set type
      QString cmd = QString("/sbin/sfdisk %1 -c %2 83").arg(homedev.mid(0,8)).arg(homedev.mid(8));
      system(cmd.toAscii());
      system("sleep 1");
      if (!makeLinuxPartition(homedev, type, badblocksCheck->isChecked())) {
        return false;
      }
      system("sleep 1");
      if (!mountPartition(homedev, "/mnt/antiX/home")) {
        return false;
      }
      isHomeFormatted = true;
    }
  }
  // mount all swaps
  system("sleep 1");
  system("/usr/sbin/buildfstab -r");
  system("/sbin/swapon -a 2>&1");

  return true;
}

void MInstall::installLinux() {
  char line[130];

  QString drv = QString("/dev/%1").arg(diskCombo->currentText());

  strcpy(line, rootCombo->currentText().toAscii());
  char *tok = strtok(line, " -");
  QString rootdev = QString("/dev/%1").arg(tok);

  strcpy(line, homeCombo->currentText().toAscii());
  tok = strtok(line, " -");
  QString homedev = QString("/dev/%1").arg(tok);

  // maybe root was formatted
  if (isRootFormatted) {
    // yes it was
    copyLinux();
  } else {
    // no--it's being reused
    updateStatus(tr("Mounting the / (root) partition"), 3);
    mountPartition(rootdev, "/mnt/antiX");
    // set all connections in advance
    disconnect(timer, SIGNAL(timeout()), 0, 0);
    connect(timer, SIGNAL(timeout()), this, SLOT(delTime()));
    disconnect(proc, SIGNAL(started()), 0, 0);
    connect(proc, SIGNAL(started()), this, SLOT(delStart()));
    disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), 0, 0);
    connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(delDone(int, QProcess::ExitStatus)));
    // setup and start the process
    QString cmd = QString("/bin/rm -r -d /mnt/antiX/bin /mnt/antiX/boot");
    cmd.append(" /mnt/antiX/dev /mnt/antiX/etc /mnt/antiX/lib /mnt/antiX/mnt");
    cmd.append(" /mnt/antiX/opt /mnt/antiX/proc /mnt/antiX/root /mnt/antiX/sbin");
    cmd.append(" /mnt/antiX/selinux /mnt/antiX/sys /mnt/antiX/tmp /mnt/antiX/usr");
    cmd.append(" /mnt/antiX/var /mnt/antiX/lib32 /mnt/antiX/lib64 /mnt/antiX/emul");
    proc->start(cmd);
  }
}

void MInstall::copyLinux() {
  char line[130];

  QString drv = QString("/dev/%1").arg(diskCombo->currentText());

  strcpy(line, rootCombo->currentText().toAscii());
  char *tok = strtok(line, " -");
  QString rootdev = QString("/dev/%1").arg(tok);

  strcpy(line, homeCombo->currentText().toAscii());
  tok = strtok(line, " -");
  QString homedev = QString("/dev/%1").arg(tok);

  // make empty dirs for opt, proc, sys, usr, mnt, mnt/temp
  // home already done
  updateStatus(tr("Creating system directories"), 9);
  mkdir("/mnt/antiX/opt", 0755);
  mkdir("/mnt/antiX/proc", 0755);
  mkdir("/mnt/antiX/sys", 0755);
  mkdir("/mnt/antiX/selinux", 0755);

  // copy most except usr, mnt and home
  // must copy boot even if saving, the new files are required
  // media is already ok, usr will be done next, home will be done later
  // set all connections in advance
  disconnect(timer, SIGNAL(timeout()), 0, 0);
  connect(timer, SIGNAL(timeout()), this, SLOT(copyTime()));
  disconnect(proc, SIGNAL(started()), 0, 0);
  connect(proc, SIGNAL(started()), this, SLOT(copyStart()));
  disconnect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), 0, 0);
  connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(copyDone(int, QProcess::ExitStatus)));
    // setup and start the process
  QString cmd = QString("/bin/cp -a /aufs/bin /aufs/boot /aufs/dev");
  cmd.append(" /aufs/etc /aufs/lib /aufs/media /aufs/mnt");
  cmd.append(" /aufs/opt /aufs/root /aufs/sbin /aufs/usr");
  cmd.append(" /aufs/var /aufs/lib32 /aufs/emul /mnt/antiX");
  system("cp -a /aufs/usr/share/antiX-install /mnt/antiX");
  proc->start(cmd);
}

///////////////////////////////////////////////////////////////////////////
// install loader

bool MInstall::makeGrub(int rootdev, QString rootpart, const char *rootmnt, bool initrd) {
  char line[130];
  char vga[20] = "vga=791 ";
  char acpi[20] = "";
  char nousb[20] = "";
  char nofloppy[20] = "";
  char generic[20] = "";

  // convert xdxn to (hdi,j)
  QString cmd = QString("/bin/grep '%1' /mnt/antiX/boot/grub/device.map").arg(rootpart.mid(0,3));
  QString val = getCmdOut(cmd.toAscii());
  QString groot = QString ("root %1,%2)\n").arg(val.mid(0,4)).arg(atoi(rootpart.mid(3).toAscii()) - 1);

//  QString swappart = QString(swapCombo->currentText()).section(" ", 0, 0);

  FILE *fp = popen("cat /proc/cmdline 2>/dev/null", "r");
  if (fp != NULL) {
    fgets(line, sizeof line, fp);
    int i = strlen(line);
    line[--i] = '\0';
    if (i > 2) {
      char *tok = strtok(line, " ");
      while (tok != NULL) {
        if (strncmp(tok, "vga", 3) == 0) {
          // override vga
          strcpy(vga, tok);
          strcat(vga, " ");
        } else if (strncmp(tok, "nofloppy", 8) == 0) {
          // override floppy
          strcpy(nofloppy, "nofloppy ");
        } else if (strncmp(tok, "all-generic-ide", 15) == 0) {
          // override all-generic-ide
          strcpy(generic, "all-generic-ide ");
        } else if (strncmp(tok, "nousb", 5) == 0) {
          // override usb
          strcpy(nousb, "nousb ");
        } else if (strncmp(tok, "acpi=off", 8) == 0 || strncmp(tok, "noacpi", 6) == 0) {
          // no acpi, use apm
          strcpy(acpi, "acpi=off apm=power_off noacpi ");
        }
        tok = strtok(NULL, " ");
      }
    }
   pclose(fp);
  }

  // create the menu.lst
  sprintf(line, "%s/boot/grub/menu.lst", rootmnt);
  fp = fopen(line, "w");
  if (fp != NULL) {
    // head
    if (!getCmdValue("cat /etc/default/antiX","SERVER","="," ").contains("yes", Qt::CaseInsensitive)) {
      fputs("timeout 10\n", fp);
    } else  {
      fputs("timeout 5\n", fp);
    }
    fputs("color cyan/blue white/blue\n", fp);
    fputs("foreground ffffff\n", fp);
    fputs("background 0639a1\n\n", fp);
    if (!getCmdValue("cat /etc/default/antiX","SERVER","="," ").contains("yes", Qt::CaseInsensitive)) {
      fputs("gfxmenu /boot/grub/message\n\n", fp);
    }

    QStringList vals = getCmdOuts("ls /mnt/antiX/boot | grep 'vmlinuz-'");
    if (vals.empty()) {
      fclose(fp);
      return false;
    }

    chdir("/mnt/antiX");

    for (QStringList::Iterator it = vals.end(); ;) {
      val = QString( tr("title Swift Linux at %1, kernel %2\n")).arg(rootpart).arg((*--it).mid(8));
      fputs(val.toAscii(), fp);
      fputs(groot.toAscii(), fp);
      if (!getCmdValue("cat /etc/default/antiX","SERVER","="," ").contains("yes", Qt::CaseInsensitive)) {
        val = QString("kernel /boot/%1 root=/dev/%2 nomce quiet nosplash nouveau.modeset=0 ").arg(*it).arg( rootpart);
      } else {
        val = QString("kernel /boot/%1 root=/dev/%2 nomce quiet nouveau.modeset=0 ").arg(*it).arg( rootpart);
      }
      fputs(val.toAscii(), fp);
      fputs(vga, fp);
      fputs(acpi, fp);
      fputs(nousb, fp);
      if (!getCmdValue("cat /etc/default/antiX","SERVER","="," ").contains("yes", Qt::CaseInsensitive)) {
      }
      fputs(nofloppy, fp);
      fputs(generic, fp);

      if (initrd) {
        cmd = QString("ls /mnt/antiX/boot | grep 'initrd.img-%1'").arg( (*it).mid(8));
        val = getCmdOut(cmd.toAscii());
        if (!val.isEmpty()) {
          cmd = QString("\ninitrd /boot/initrd.img-%1").arg((*it).mid(8));
          fputs(cmd.toAscii(), fp);
        }
      }
      fputs("\nboot\n\n", fp);

      if (it == vals.begin()) {
        break;
      }
    }
    
    // add windows entries the new way
    QStringList strings = getCmdOuts("/usr/bin/os-prober | grep chain");

    for (QStringList::Iterator it = strings.begin(); it != strings.end(); ++it) {
      QString chain = *it;
      QString chainDev = chain.section(":", 0, 0).section("/",2,2);
      QString chainName = chain.section(":", 1, 1);
      val = QString(tr("title %1 at %2\n")).arg(chainName).arg(chainDev);
      fputs(val.toAscii(), fp);

      // convert xdxn to (hdi,j)
      cmd = QString("/bin/grep '%1' /mnt/antiX/boot/grub/device.map").arg(chainDev.mid(0,3));
      val = getCmdOut(cmd.toAscii());

      //if not hd0
      if (val.mid(0,5).compare("(hd0)") != 0) {
        cmd = QString ("map (hd0) %1\n").arg(val.mid(0,5));
        fputs(cmd.toAscii(), fp);
        cmd = QString ("map %1 (hd0)\n").arg(val.mid(0,5));
        fputs(cmd.toAscii(), fp);
      }

      cmd = QString ("rootnoverify %1,%2)\n").arg(val.mid(0,4)).arg(atoi(chainDev.mid(3).toAscii()) - 1);
      fputs(cmd.toAscii(), fp);
      fputs("chainloader +1\n\n", fp);
    }

    // add other linux entries the new way
    val = QString("/usr/bin/os-prober | grep linux | grep -v %1").arg(rootpart);
    strings = getCmdOuts(val.toAscii());
    for (QStringList::Iterator it = strings.begin(); it != strings.end(); ++it) {
      QString lin = *it;
      QString linDev = lin.section(":", 0, 0);
      cmd = QString("/usr/bin/linux-boot-prober %1 | grep 'vmlinuz'").arg(linDev);
      lin = getCmdOut(cmd.toAscii());
      if (!lin.isEmpty()) {
        QString val2 = lin.section(":", 2, 2);
        val = QString("title %1\n").arg(val2);
        fputs(val.toAscii(), fp);
        linDev = lin.section(":", 0, 0).section("/",2,2);
        // convert xdxn to (hdi,j)
        cmd = QString("/bin/grep '%1' /mnt/antiX/boot/grub/device.map").arg(linDev.mid(0,3));
        val = getCmdOut(cmd.toAscii());
        cmd = QString ("root %1,%2)\n").arg(val.mid(0,4)).arg(atoi(linDev.mid(3).toAscii()) - 1);
        fputs(cmd.toAscii(), fp);
        val2 = lin.section(":", 3, 3);
        QString val3 = lin.section(":", 5, 5);
        val = QString("kernel %1 %2\n").arg(val2).arg(val3);
        fputs(val.toAscii(), fp);
        val2 = lin.section(":", 4, 4);
        if (!val2.isEmpty()) {
          val = QString("initrd %1\n").arg(val2);
          fputs(val.toAscii(), fp);
        }
      }
      fputs("\n", fp);
    }

    // memtest
   // fputs("title MEMTEST\n", fp);
   // fputs("kernel /boot/memtest86+.bin\n\n", fp);


    fclose(fp);
  } else {
    return false;
  }
  return true;

}

// build a grub configuration and install grub
bool MInstall::installLoader() {
  QString cmd;
  QString val = getCmdOut("ls /mnt/antiX/boot | grep 'initrd.img-2.6'");

  // the old initrd is not valid for this hardware
  if (!val.isEmpty()) {
    cmd = QString("rm -f /mnt/antiX/boot/%1").arg(val);
    system(cmd.toAscii());
  }

  // maybe replace the initrd.img file
  if (initrdCheck->isChecked()) {
      setCursor(QCursor(Qt::WaitCursor));
      system("chroot /mnt/antiX mount /proc");
      cmd = QString("chroot /mnt/antiX mkinitramfs -o /boot/%1").arg(val);
      system(cmd.toAscii());
      system("chroot /mnt/antiX umount /proc");
      setCursor(QCursor(Qt::ArrowCursor));
  }

  if (!grubCheckBox->isChecked()) {
    // skip it
    return true;
  }

  QString bootdrv = QString(grubBootCombo->currentText()).section(" ", 0, 0);
  QString rootpart = QString(rootCombo->currentText()).section(" ", 0, 0);
  QString boot;
  int rootdev = diskCombo->currentIndex();


  if (grubMbrButton->isChecked()) {
    boot = bootdrv;
  } else {
    boot = rootpart;
  }

  // install Grub?
  QString msg = QString( tr("Ok to install GRUB bootloader at %1 ?")).arg(boot);
  int ans = QMessageBox::warning(this, QString::null, msg,
        tr("Yes"), tr("No"));
  if (ans != 0) {
    return false;
  }
  setCursor(QCursor(Qt::WaitCursor));

  // install new Grub now
  cmd = QString("grub-install --no-floppy --root-directory=/mnt/antiX /dev/%1").arg(boot);
  if (system(cmd.toAscii()) != 0) {
    // error, try again
    // this works for reiser-grub bug
    if (system(cmd.toAscii()) != 0) {
      // error
      setCursor(QCursor(Qt::ArrowCursor));
      QMessageBox::critical(this, QString::null,
        tr("Sorry, installing GRUB failed. This may be due to a change in the disk formatting. You can uncheck GRUB and finish installing Swift Linux, then reboot to the CD and repair the installation with the reinstall GRUB function."));
      return false;
    }
  }
 
  // make new menu.1st file
  if (!makeGrub(rootdev, rootpart, "/mnt/antiX", initrdCheck->isChecked())) {
    setCursor(QCursor(Qt::ArrowCursor));
    QMessageBox::critical(this, QString::null,
      tr("Sorry, creating menu.lst failed. Root filesystem may be faulty."));
    return false;
  } else {
    QMessageBox::information(this, QString::null, tr("GRUB installed ok."));
  }
  setCursor(QCursor(Qt::ArrowCursor));
  return true;
}

/////////////////////////////////////////////////////////////////////////
// create the user, can not be rerun

bool MInstall::setUserName() {
  int ans;
  DIR *dir;
  QString msg, cmd;

  // see if user directory already exists
  QString dpath = QString("/mnt/antiX/home/%1").arg(userNameEdit->text());
  if ((dir = opendir(dpath.toAscii())) != NULL) {
    // already exists
    closedir(dir);
    msg = QString( tr("The home directory for %1 already exists.Would you like to reuse the old home directory?")).arg(userNameEdit->text());
    ans = QMessageBox::information(0, QString::null, msg,
        tr("Yes"), tr("No"));
    if (ans != 0) {
      // don't reuse -- maybe save the old home
      msg = QString( tr("Would you like to save the old home directory\nand create a new home directory?"));
      ans = QMessageBox::information(0, QString::null, msg,
        tr("Yes"), tr("No"));
      if (ans == 0) {
        // save the old directory
        cmd = QString("mv -f %1 %2.001").arg(dpath).arg(dpath);
        if (system(cmd.toAscii()) != 0) {
          cmd = QString("mv -f %1 %2.002").arg(dpath).arg(dpath);
          if (system(cmd.toAscii()) != 0) {
            cmd = QString("mv -f %1 %2.003").arg(dpath).arg(dpath);
            if (system(cmd.toAscii()) != 0) {
              cmd = QString("mv -f %1 %2.004").arg(dpath).arg(dpath);
              if (system(cmd.toAscii()) != 0) {
                cmd = QString("mv -f %1 %2.005").arg(dpath).arg(dpath);
                if (system(cmd.toAscii()) != 0) {
                  QMessageBox::critical(0, QString::null,
                    tr("Sorry, failed to save old home directory. Before proceeding,\nyou'll have to select a different username or\ndelete a previously saved copy of your home directory."));
                  return false;
                }
              }
            }
          }
	      }	
      } else {
        // don't save and don't reuse -- delete?
        msg = QString( tr("Would you like to delete the old home directory for %1?")).arg(userNameEdit->text());
        ans = QMessageBox::information(0, QString::null, msg,
            tr("Yes"), tr("No"));
        if (ans == 0) {
          // delete the directory
          cmd = QString("rm -f %1").arg(dpath);
          if (system(cmd.toAscii()) != 0) {
            QMessageBox::critical(0, QString::null,
              tr("Sorry, failed to delete old home directory. Before proceeding, \nyou'll have to select a different username."));
            return false;
          }
        } else {  
	        // don't save, reuse or delete -- can't proceed
	        QMessageBox::critical(0, QString::null,
            tr("You've chosen to not use, save or delete the old home directory.\nBefore proceeding, you'll have to select a different username."));
          return false;
      	}
      }
    }
  }

  if ((dir = opendir(dpath.toAscii())) == NULL) {
    // dir does not exist, must create it
    // copy skel to demo
    if (system("cp -a /mnt/antiX/etc/skel /mnt/antiX/home") != 0) {
      QMessageBox::critical(0, QString::null,
        tr("Sorry, failed to create user directory."));
      return false;
    }
    cmd = QString("mv -f /mnt/antiX/home/skel %1").arg(dpath);
    if (system(cmd.toAscii()) != 0) {
      QMessageBox::critical(0, QString::null,
        tr("Sorry, failed to name user directory."));
      return false;
    }
  } else {
    // dir does exist, clean it up
    cmd = QString("cp /mnt/antiX/etc/skel/.bash_profile %1").arg(dpath);
    system(cmd.toAscii());
    cmd = QString("cp /mnt/antiX/etc/skel/.bashrc %1").arg(dpath);
    system(cmd.toAscii());
    cmd = QString("cp /mnt/antiX/etc/skel/.gtkrc %1").arg(dpath);
    system(cmd.toAscii());
    cmd = QString("cp /mnt/antiX/etc/skel/.gtkrc-2.0 %1").arg(dpath);
    system(cmd.toAscii());
  }
  // fix the ownership, demo=newuser
  cmd = QString("chown -R demo.users %1").arg(dpath);
  if (system(cmd.toAscii()) != 0) {
    QMessageBox::critical(0, QString::null,
      tr("Sorry, failed to set ownership of user directory."));
    return false;
  }

  // change in files
  replaceStringInFile("demo", userNameEdit->text(), "/mnt/antiX/etc/aliases");
  replaceStringInFile("demo", userNameEdit->text(), "/mnt/antiX/etc/group");
  replaceStringInFile("demo", userNameEdit->text(), "/mnt/antiX/etc/gshadow");
  replaceStringInFile("demo", userNameEdit->text(), "/mnt/antiX/etc/passwd");
  replaceStringInFile("demo", userNameEdit->text(), "/mnt/antiX/etc/shadow");
  replaceStringInFile("demo", userNameEdit->text(), "/mnt/antiX/etc/samba/smb.conf");
//  replaceStringInFile("demo", userNameEdit->text(), "/mnt/antiX/etc/apache2/sites-enabled/webserver1.conf");
  replaceStringInFile("demo", userNameEdit->text(), "/mnt/antiX/var/lib/kdm/kdmsts");

  cmd = QString("touch /mnt/antiX/var/mail/%1").arg(userNameEdit->text());
  system(cmd.toAscii());

  return true;
}

bool MInstall::setPasswords() {
  FILE *fp = popen("chroot /mnt/antiX passwd root", "w");
  bool fpok = true;
  QString cmd = QString("%1\n").arg(rootPasswordEdit->text());
  if (fp != NULL) {
    sleep(6);
    if (fputs(cmd.toAscii(), fp) >= 0) {
      fflush(fp);
      sleep(2);
      if (fputs(cmd.toAscii(), fp) < 0) {
        fpok = false;
      }
      fflush(fp);
    } else {
      fpok = false;
    }
    pclose(fp);
  } else {
    fpok = false;
  }

  if (!fpok) {
    QMessageBox::critical(0, QString::null,
      tr("Sorry, unable to set root password."));
    return false;
  }
//  cmd = QString("chroot /mnt/antiX /usr/share/webmin/changepass.pl /etc/webmin root %1").arg(rootPasswordEdit->text());
//  system(cmd.toAscii());

  fp = popen("chroot /mnt/antiX passwd demo", "w");
  fpok = true;
  cmd = QString("%1\n").arg(userPasswordEdit->text());
  if (fp != NULL) {
    sleep(1);
    if (fputs(cmd.toAscii(), fp) >= 0) {
      fflush(fp);
      sleep(1);
      if (fputs(cmd.toAscii(), fp) < 0) {
        fpok = false;
      }
      fflush(fp);
    } else {
      fpok = false;
    }
    pclose(fp);
  } else {
    fpok = false;
  }

  if (!fpok) {
    QMessageBox::critical(0, QString::null,
      tr("Sorry, unable to set user password."));
    return false;
  }
  return true;
}

bool MInstall::setUserInfo() {

  //validate data before proceeding
  // see if username is reasonable length
  if (strlen(userNameEdit->text().toAscii()) < 2) {
    QMessageBox::critical(0, QString::null,
      tr("The user name needs to be at least\n"
      "2 characters long. Please select\n"
      "a longer name before proceeding."));
    return false;
  }
  if (strlen(userPasswordEdit->text().toAscii()) < 2) {
    QMessageBox::critical(0, QString::null,
      tr("The user password needs to be at least\n"
      "2 characters long. Please select\n"
      "a longer password before proceeding."));
    return false;
  }
  if (strlen(rootPasswordEdit->text().toAscii()) < 2) {
    QMessageBox::critical(0, QString::null,
      tr("The root password needs to be at least\n"
      "2 characters long. Please select\n"
      "a longer password before proceeding."));
    return false;
  }
  // check that user name is not already used
  QString cmd = QString("grep '^%1' /etc/passwd >/dev/null").arg(userNameEdit->text());
  if (system(cmd.toAscii()) == 0) {
        QMessageBox::critical(0, QString::null,
            tr("Sorry that name is in use.\n"
              "Please select a different name.\n"));
      return false;
  }
        
  if (strcmp(userPasswordEdit->text().toAscii(), userPasswordEdit2->text().toAscii()) != 0) {
    QMessageBox::critical(0, QString::null,
      tr("The user password entries do\n"
        "not match.  Please try again."));
    return false;
  }
  if (strcmp(rootPasswordEdit->text().toAscii(), rootPasswordEdit2->text().toAscii()) != 0) {
    QMessageBox::critical(0, QString::null,
      tr("The root password entries do\n"
        " not match.  Please try again."));
    return false;
  }
  if (strlen(userPasswordEdit->text().toAscii()) < 2) {
    QMessageBox::critical(0, QString::null,
      tr("The user password needs to be at least\n"
      "2 characters long. Please select\n"
      "a longer password before proceeding."));
    return false;
  }
  if (strlen(rootPasswordEdit->text().toAscii()) < 2) {
    QMessageBox::critical(0, QString::null,
      tr("The root password needs to be at least\n"
      "2 characters long. Please select\n"
      "a longer password before proceeding."));
    return false;
  }

  if (!setPasswords()) {
    return false;
  }
  return setUserName();
}

/////////////////////////////////////////////////////////////////////////
// set the computer name, can not be rerun

bool MInstall::setComputerName() {

  // see if name is reasonable
  if (computerNameEdit->text().length() < 2) {
    QMessageBox::critical(0, QString::null,
      tr("Sorry your computer name needs to be\nat least 2 characters long. You'll have to\nselect a different name before proceeding."));
    return false;
  }
  // see if name is reasonable
  if (computerDomainEdit->text().length() < 2) {
    QMessageBox::critical(0, QString::null,
      tr("Sorry your computer domain needs to be at least\n2 characters long. You'll have to select a different\nname before proceeding."));
    return false;
  }

  QString val = getCmdValue("dpkg -s samba | grep '^Status'", "ok", " ", " ");
  if (val.compare("installed") == 0) {
    // see if name is reasonable
    if (computerGroupEdit->text().length() < 2) {
      QMessageBox::critical(0, QString::null,
        tr("Sorry your workgroup needs to be at least\n2 characters long. You'll have to select a different\nname before proceeding."));
      return false;
    }
    replaceStringInFile("antiX1", computerNameEdit->text(), "/mnt/antiX/etc/samba/smb.conf");
    replaceStringInFile("antiXgrp", computerGroupEdit->text(), "/mnt/antiX/etc/samba/smb.conf");
  }
  if (sambaCheckBox->isChecked()) {
    system("mv -f /mnt/antiX/etc/rc5.d/K20samba /mnt/antiX/etc/rc5.d/S20samba >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/K20samba /mnt/antiX/etc/rc4.d/S20samba >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/K20samba /mnt/antiX/etc/rc3.d/S20samba >/dev/null 2>&1");
  } else {
    system("mv -f /mnt/antiX/etc/rc5.d/S20samba /mnt/antiX/etc/rc5.d/K20samba >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/S20samba /mnt/antiX/etc/rc4.d/K20samba >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/S20samba /mnt/antiX/etc/rc3.d/K20samba >/dev/null 2>&1");
  }

  replaceStringInFile("antiX1", computerNameEdit->text(), "/mnt/antiX/etc/hosts");

  QString cmd = QString("echo \"%1\" | cat > /mnt/antiX/etc/hostname").arg(computerNameEdit->text());
  system(cmd.toAscii());
  cmd = QString("echo \"%1\" | cat > /mnt/antiX/etc/mailname").arg(computerNameEdit->text());
  system(cmd.toAscii());
  cmd = QString("sed -i 's/.*send host-name.*/send host-name \"%1\";/g' /mnt/antiX/etc/dhcp3/dhclient.conf").arg(computerNameEdit->text());
  system(cmd.toAscii());
  cmd = QString("echo \"%1\" | cat > /mnt/antiX/etc/defaultdomain").arg(computerDomainEdit->text());
  system(cmd.toAscii());
//  cmd = QString("mv -f /mnt/antiX/etc/bind/example.dom.hosts /mnt/antiX/etc/bind/%1.hosts").arg(computerDomainEdit->text());
//  system(cmd.toAscii());

  return true;
}

void MInstall::setLocale() {
  QString cmd2;
  QString kb = keyboardCombo->currentText();
  //keyboard
  QString cmd = QString("chroot /mnt/antiX /usr/sbin/install-keymap \"%1\"").arg(kb);
  system(cmd.toAscii());
  if (kb == "uk") {
    kb = "gb";
  }
  //cmd = QString("sed -i 's/XKBLAYOUT.*/XKBLAYOUT=\"%1\"/g' /etc/default/keyboard").arg(kb);
  //system(cmd.toAscii());

  //locale
  cmd = QString("chroot /mnt/antiX /usr/sbin/update-locale \"LANG=%1\"").arg(localeCombo->currentText());
  system(cmd.toAscii());
  cmd = QString("Language=%1").arg(localeCombo->currentText());
  replaceStringInFile("Language=.*", cmd, "/mnt/antiX/etc/kde3/kdm/kdmrc");

  // timezone
  system("cp -f /etc/default/rcS /mnt/antiX/etc/default");
  system("cp -f /etc/adjtime /mnt/antiX/etc/");
  // /etc/localtime is a copy of one of the timezone files in /usr/share/zoneinfo. Use the one selected by the user.
  cmd = QString("cp -f /usr/share/zoneinfo/%1 /mnt/antiX/etc/localtime").arg(timezoneCombo->currentText());
  system(cmd.toAscii());
  // /etc/timezone is text file with the timezone written in it. Write the user-selected timezone in it now.
  cmd = QString("echo %1 > /mnt/antiX/etc/timezone").arg(timezoneCombo->currentText());
  system(cmd.toAscii());

  if (gmtCheckBox->isChecked()) {
    replaceStringInFile("^UTC=no", "UTC=yes", "/mnt/mepis/etc/default/rcS");
}

}

void MInstall::setServices() {
  setCursor(QCursor(Qt::WaitCursor));
  if (guarddogItem != NULL) {
    if (guarddogItem->checkState(0) == Qt::Checked) {
      replaceStringInFile("# DISABLED=1", "# DISABLED=0", "/mnt/antiX/etc/rc.firewall");
      replaceStringInFile("DISABLE_GUARDDOG=1", "DISABLE_GUARDDOG=0", "/mnt/antiX/etc/rc.firewall");
    } else {
      replaceStringInFile("# DISABLED=0", "# DISABLED=1", "/mnt/antiX/etc/rc.firewall");
      replaceStringInFile("DISABLE_GUARDDOG=0", "DISABLE_GUARDDOG=1", "/mnt/antiX/etc/rc.firewall");
    }
  }

  if (cpufreqItem != NULL && cpufreqItem->checkState(0) == Qt::Checked) {
    system("mv -f /mnt/antiX/etc/rc5.d/K97cpufrequtils /mnt/antiX/etc/rc5.d/S03cpufrequtils >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/K97cpufrequtils /mnt/antiX/etc/rc4.d/S03cpufrequtils >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/K97cpufrequtils /mnt/antiX/etc/rc3.d/S03cpufrequtils >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/K97cpufrequtils /mnt/antiX/etc/rc2.d/S03cpufrequtils >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc5.d/K98loadcpufreq /mnt/antiX/etc/rc5.d/S02loadcpufreq >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/K98loadcpufreq /mnt/antiX/etc/rc4.d/S02loadcpufreq >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/K98loadcpufreq /mnt/antiX/etc/rc3.d/S02loadcpufreq >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/K98loadcpufreq /mnt/antiX/etc/rc2.d/S02loadcpufreq >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc5.d/K01irqbalance /mnt/antiX/etc/rc5.d/S02irqbalance >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/K01irqbalance /mnt/antiX/etc/rc4.d/S02irqbalance >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/K01irqbalance /mnt/antiX/etc/rc3.d/S02irqbalance >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/K01irqbalance /mnt/antiX/etc/rc2.d/S02irqbalance >/dev/null 2>&1");
  } else {
    system("mv -f /mnt/antiX/etc/rc5.d/S03cpufrequtils /mnt/antiX/etc/rc5.d/K97cpufrequtils >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/S03cpufrequtils /mnt/antiX/etc/rc4.d/K97cpufrequtils >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/S03cpufrequtils /mnt/antiX/etc/rc3.d/K97cpufrequtils >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/S03cpufrequtils /mnt/antiX/etc/rc2.d/K97cpufrequtils >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc5.d/S02loadcpufreq /mnt/antiX/etc/rc5.d/K98loadcpufreq >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/S02loadcpufreq /mnt/antiX/etc/rc4.d/K98loadcpufreq >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/S02loadcpufreq /mnt/antiX/etc/rc3.d/K98loadcpufreq >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/S02loadcpufreq /mnt/antiX/etc/rc2.d/K98loadcpufreq >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc5.d/S02irqbalance /mnt/antiX/etc/rc5.d/K01irqbalance >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/S02irqbalance /mnt/antiX/etc/rc4.d/K01irqbalance >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/S02irqbalance /mnt/antiX/etc/rc3.d/K01irqbalance >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/S02irqbalance /mnt/antiX/etc/rc2.d/K01irqbalance >/dev/null 2>&1");
  }
    
  if (wicdItem != NULL && wicdItem->checkState(0) == Qt::Checked) {
    system("mv -f /mnt/antiX/etc/rc5.d/K03wicd /mnt/antiX/etc/rc5.d/S03wicd >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/K03wicd /mnt/antiX/etc/rc4.d/S03wicd >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/K03wicd /mnt/antiX/etc/rc3.d/S03wicd >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/K03wicd /mnt/antiX/etc/rc2.d/S03wicd >/dev/null 2>&1");
  } else {
    system("mv -f /mnt/antiX/etc/rc5.d/S03wicd /mnt/antiX/etc/rc5.d/K03wicd >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/S03wicd /mnt/antiX/etc/rc4.d/K03wicd >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/S03wicd /mnt/antiX/etc/rc3.d/K03wicd >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/S03wicd /mnt/antiX/etc/rc2.d/K03wicd >/dev/null 2>&1");
  }

  if (cupsItem != NULL && cupsItem->checkState(0) == Qt::Checked) {
    system("mv -f /mnt/antiX/etc/rc5.d/K02cups /mnt/antiX/etc/rc5.d/S02cups >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/K02cups /mnt/antiX/etc/rc4.d/S02cups >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/K02cups /mnt/antiX/etc/rc3.d/S02cups >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/K02cups /mnt/antiX/etc/rc2.d/S02cups >/dev/null 2>&1");
  } else {
    system("mv -f /mnt/antiX/etc/rc5.d/S02cups /mnt/antiX/etc/rc5.d/K02cups >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/S02cups /mnt/antiX/etc/rc4.d/K02cups >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/S02cups /mnt/antiX/etc/rc3.d/K02cups >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/S02cups /mnt/antiX/etc/rc2.d/K02cups >/dev/null 2>&1");
  } 

  if (laptopItem != NULL && laptopItem->checkState(0) == Qt::Checked) {
    system("mv -f /mnt/antiX/etc/rc5.d/K01laptop-mode /mnt/antiX/etc/rc5.d/S05laptop-mode >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/K01laptop-mode /mnt/antiX/etc/rc4.d/S05laptop-mode >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/K01laptop-mode /mnt/antiX/etc/rc3.d/S05laptop-mode >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/K01laptop-mode /mnt/antiX/etc/rc2.d/S05laptop-mode >/dev/null 2>&1");
  } else {
    system("mv -f /mnt/antiX/etc/rc5.d/S05laptop-mode /mnt/antiX/etc/rc5.d/K01laptop-mode >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc4.d/S05laptop-mode /mnt/antiX/etc/rc4.d/K01laptop-mode >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc3.d/S05laptop-mode /mnt/antiX/etc/rc3.d/K01laptop-mode >/dev/null 2>&1");
    system("mv -f /mnt/antiX/etc/rc2.d/S05laptop-mode /mnt/antiX/etc/rc2.d/K01laptop-mode >/dev/null 2>&1");
  } 

  setCursor(QCursor(Qt::ArrowCursor));
}

void MInstall::stopInstall() {
  int curr = widgetStack->currentIndex();
  int c = widgetStack->count();

  if (curr == 3) {
    procAbort();
    QApplication::beep();
    return;
  } else if (curr >= c-3) {
    int ans = QMessageBox::information(0, QString::null,
      tr("Swift Linux installation and configuration is complete.\n"
        "To use the new installation, reboot without the CD.\n"
        "Do you want to reboot now?"),
        tr("Yes"), tr("No"));
    if (ans == 0) {
      system("/bin/umount -l /mnt/antiX/home >/dev/null 2>&1");
      system("/bin/umount -l /mnt/antiX >/dev/null 2>&1");
      system("init 6");
      return;
    }

  } else if (curr > 3) {
    int ans = QMessageBox::critical(0, QString::null,
      tr("The installation and configuration is incomplete.\nDo you really want to stop now?"),
        tr("Yes"), tr("No"));
    if (ans != 0) {
      return;
    }
  }
  system("/bin/umount -l /mnt/antiX/home >/dev/null 2>&1");
  system("/bin/umount -l /mnt/antiX >/dev/null 2>&1");
}

void MInstall::unmountGoBack(QString msg) {
  system("/bin/umount -l /mnt/antiX/home >/dev/null 2>&1");
  system("/bin/umount -l /mnt/antiX >/dev/null 2>&1");
  goBack(msg);
}

void MInstall::goBack(QString msg) {
  QMessageBox::critical(0, QString::null, msg);
  gotoPage(1);
}

int MInstall::showPage(int curr, int next) {
  if (next == 1 && curr == 0) {
  } else if (next == 2 && curr == 1) {
    agreeCheckBox->setChecked(false);
    if (entireDiskButton->isChecked()) {
      return 3;
    }
  } else if (next == 3 && curr == 4) {
    return 1;
  } else if (next == 5 && curr == 4) {
    if (!installLoader()) {
      return curr;
    }
  } else if (next == 9 && curr == 8) {
    if (!setUserInfo()) {
      return curr;
    }
  } else if (next == 7 && curr == 6) {
    if (!setComputerName()) {
      return curr;
    }
  } else if (next == 8 && curr == 7) {
    setLocale();
  } else if (next == 6 && curr == 5) {
    setServices();
  }
  return next;
}

void MInstall::pageDisplayed(int next) {
  QString val;

  switch (next) {
    case 1:
      setCursor(QCursor(Qt::ArrowCursor));
      ((MMain *)mmn)->setHelpText(tr("<p><b>General Instructions</b><br/>BEFORE PROCEEDING, CLOSE ALL OTHER APPLICATIONS.</p>"
        "<p>On each page, please read the instructions, make your selections, and then click on Next when you are ready to proceed. "
        "You will be prompted for confirmation before any destructive actions are performed.</p>"
        "<p>Swift Linux requires about 2 GB of space. 5 GB or more is preferred."
        "You can use the entire disk or you can put Swift Linux on existing partitions. </p>"
        "<p>If you are using PC type hardware, run GParted from here if you need to modify some partitions before doing a custom install. If you are using Apple hardware, you must never use parted or GParted on your boot drive. Instead you must setup your partitions and boot manager in OSX before installing Swift Linux. The SimplyMEPIS Assistant is an OSX application available on the Swift Linux CD to help you prepare your OSX boot volume for Swift Linux.</p>"
        "<p>The ext3, ext4, and reiserfs Linux filesystems are supported and ext3 is recommended.</p>"
        "<p><b>Partition Requirements</b><br/>A linux-swap partition is highly recommended. "
        "Your RAM memory plus swap space must be at least 128MB. "
        "A larger size can improve system performance but you should not need more than 512MB of swap space unless you develop software or edit video files or run a database server.</p>"
        "<p>A separate linux /home (pronounced home) partition is recommended and should be at least 200MB and preferably as large as possible. "
        "This is where your work will be stored. "
        "A separate home partition will make it easier to backup, upgrade or reinstall Swift Linux in the future."
        "A linux / ( pronounced root) partition is required. "
        "It needs to be at least 1.3 GB for Swift Linux and it must be larger if you do not have a separate home partition. "
        "This is where additional applications will be stored and where the Linux kernel and drivers may be compiled. "
        "If you will be installing and/or testing a lot of applications, you should make this partition larger but, unless you test a lot software, it is unlikely you would need more than 4GB in this partition.</p>"
        "<p><b>Auto-install Using Entire Disk</b><br/>The selected disk will be reformatted and the installer will partition the disk as Swift Linux prefers. "
        "Optionally you may request that a portion of the disk is left free if possible, for example so you can install a second OS later.</p>"
        "<p><b>Custom Install on Existing Partitions</b><br/>Swift Linux will be installed on the existing partitions you choose. "
        "If the disk isn't already partitioned appropriately, you can modify the partitions here with GParted.  If you have modified the partitions, it is best to reboot the system before continuing with the installation.  Do NOT use parted or GParted if you are installing on an Apple computer boot drive.</p>"
        "<p><b>Upgrading</b><br/>To upgrade an existing Linux installation, choose to install on existing partitions and then choose to preserve the data in /home.</p>"));
      break;
    
    case 2:
      ((MMain *)mmn)->setHelpText(tr("<p><b>Limitations</b><br/>Remember, this software is provided AS-IS with no warranty what-so-ever. "
         "It's solely your responsibility to backup your data before proceeding.</p>"
         "<p><b>Choose Partitions</b><br/>Swift Linux requires a root partition. The swap partition is optional but highly recommended. If you want to use the Suspend-to-Disk feature of Swift Linux, you will need a swap partition that is larger than your physical memory size.</p>"
         "<p>If you choose a separate /home partition it will be easier for you to upgrade in the future, but this will not be possible if you are upgrading from an installation that does not have a separate home partition.</p>"
         "<p><b>Upgrading</b><br/>To upgrade from an existing Linux installation, select the same home partition as before and check the preference to preserve data in /home.</p>"
         "<p>If you are preserving an existing /home directory tree located on your root partition, the installer will not reformat the root partition. "
         "As a result, the installation will take much longer than usual.</p>"
         "<p><b>Preferred Filesystem Type</b><br/>For Swift Linux, you may choose to format the partitions as ext3, ext4, or reiser. </p>"
         "<p><b>Bad Blocks</b><br/>If you choose ext3 or ext4 as the format type, you have the option of checking and correcting for badblocks on the drive. "
         "The badblock check is very time consuming, so you may want to skip this step unless you suspect that your drive has badblocks.</p>"));
      break;

    case 3:
      setCursor(QCursor(Qt::WaitCursor));
      tipsEdit->setText(tr("<p><b>Special Thanks</b><br/>My thanks to everyone who has chosen to support antiX with their time, money, suggestions, work, praise, ideas, promotion, and/or encouragement.</p>"
      "<p>Without you there would be no antiX Linux.</p>"
      "<p>anticapitalista</p>"));
      ((MMain *)mmn)->setHelpText(tr("<p><b>Installation in Progress</b><br/>"
        "Swift Linux is installing.  For a fresh install, this will probably take 3-20 minutes, depending on the speed of your system and the size of any partitions you are reformatting.</p>"
        "<p>If you click the Abort button, the installation will be stopped as soon as possible.</p>"));
      nextButton->setEnabled(false);
      prepareToInstall();
      if (entireDiskButton->isChecked()) {
        if (!makeDefaultPartitions()) {
          // failed
          system("sleep 1");
          system("/usr/sbin/buildfstab -r");
          system("/sbin/swapon -a 2>&1");
          nextButton->setEnabled(true);
          goBack(tr("Failed to create required partitions.\nReturning to Step 1."));
          break;
        }
      } else {
        if (!makeChosenPartitions()) {
          system("sleep 1");
          system("/usr/sbin/buildfstab -r");
          system("/sbin/swapon -a 2>&1");
          nextButton->setEnabled(true);
          goBack(tr("Failed to prepare chosen partitions.\nReturning to Step 1."));
          break;
        }
      }
      system("sleep 1");
      system("/usr/sbin/buildfstab -r");
      system("/sbin/swapon -a 2>&1");
      installLinux();
      break;

    case 4:
      setCursor(QCursor(Qt::ArrowCursor));
      ((MMain *)mmn)->setHelpText(tr("<p><b>Select Boot Method</b><br/>Swift Linux uses the GRUB bootloader to boot Swift Linux and MS-Windows. "
        "If you have other versions of Linux already installed on your computer, GRUB will not be automatically configured to boot them. "
        "You will have to add them manually to the /boot/grub/menu.lst file.</p>"
        "<p>If you install GRUB here, by default it is placed in the Master Boot Record of your boot drive "
        "and replaces whatever boot loader you may have been using before. This is normal.</p>"
        "<p>If you choose to install GRUB at root instead of MBR, then GRUB will be installed at the beginning of the root partition. "
        "In most cases, this will allow you to start GRUB from a third party bootloader. This option is for experts only.</p>"
        "<p>The use initrd option will allow a special software called initrd to preload extra drivers, restore from suspend to disk, and start the splash screen earlier. Its use is recommended, but not mandatory. </p>"
        "<p>If you do not select the Install GRUB checkbox, GRUB will not be installed at this time. "
        "You can install GRUB later by using the Reinstall GRUB function in the MEPIS Utilities. "
        "Reinstall GRUB will also give you the option to install GRUB on a floppy disk.</p>"));
      backButton->setEnabled(false);
      break;

    case 5:
      ((MMain *)mmn)->setHelpText(tr("<p><b>Common Services to Enable</b><br/>Select any of the these common services that you might need with your system configuration and the services will be started automatically when you start Swift Linux.</p>"));
      nextButton->setEnabled(true);
      backButton->setEnabled(true);
      break;

    case 6:
      ((MMain *)mmn)->setHelpText(tr("<p><b>Computer Identity</b><br/>The computer name is a common unique name which will identify your computer if it is on a network. "
        "The computer domain is unlikely to be used unless your ISP or local network requires it.</p>"
        "<p>The SaMBa Server needs to be activated if you want to use it to share some of your directories or printer "
        "with a local computer that is running MS-Windows or Mac OSX.</p>"));
      nextButton->setEnabled(true);
      backButton->setEnabled(false);
      break;

    case 7:
      ((MMain *)mmn)->setHelpText(tr("<p><b>Localization Defaults</b><br/>Set the default keyboard and locale.  These will apply unless, they are overridden later by the user.</p>"
        "<p><b>Configure Clock</b><br/>If you have an Apple or a pure Unix computer, by default the system clock is set to GMT or Universal Time.  In this case, check the box for 'System clock uses GMT.'</p>"
        "The CD boots with the timezone preset to EST. To change the timezone, after you reboot into the new installation, right click on the clock in the Panel and select Adjust Date & Time...</p>"));
      nextButton->setEnabled(true);
      backButton->setEnabled(false);
      break;

    case 8:
      ((MMain *)mmn)->setHelpText(tr("<p><b>Default User Login</b><br/>The root user is similar to the Administrator user in some other operating systems. "
        "You should not use the root user as your daily user account. "
        "Please enter the name for a new (default) user account that you will use on a daily basis. "
        "If needed, you can add other user accounts later. </p>"
        "<p><b>Passwords</b><br/>Enter a new password for your default user account and for the root account. "
        "Each password must be entered twice.</p>"));
      nextButton->setEnabled(true);
      break;

    case 9:
      ((MMain *)mmn)->setHelpText(tr("<p><b>Congratulations!</b><br/>You have completed the installation of Swift Linux.</p>"
        "<p><b>Finding Applications</b><br/>There are hundreds of excellent applications installed with Swift Linux. "
        "The best way to learn about them is to browse through the Menu and try them. "
        "Many of the apps were developed specifically for the icewm environment. "
        "These are shown in the main menus. "
        "Other Linux applications can be found in Menu > Applications</p>"
        "<p>In addition Swift Linux includes many standard linux applications that are run only from the commandline and therefore do not show up in Menu.</p>"));
      nextButton->setEnabled(true);
      backButton->setEnabled(false);
      break;

    default:
      // case 0 or any other
      ((MMain *)mmn)->setHelpText("<p><b>MEPIS COLLECTIVE WORK LICENSE</b></p>"
        "<p><b>COPYRIGHT</b><br/>"
        "Unless stated otherwise, MEPIS software packages are unique collective works under US copyright law.  Copyright (c) MEPIS LLC.  All rights reserved.</p>"
        "<p><b>GRANT OF COLLECTIVE WORK LICENSE</b><br/>"
        "Except as specifically stated in a separate agreement, and subject to the Terms and Conditions stated herein, MEPIS LLC hereby grants you the right to install or redistribute this collective work for non-commercial purposes for use with other legally obtained MEPIS software collections.</p>"
        "<p><b>TERMS AND CONDITIONS</b></p>"
        "<p><b>1. COMPONENT LICENSE RIGHTS</b><br/>"
        "Nothing in this collective work license limits your rights under, or grants you rights that supercede, the terms of any applicable component license. "
        "Each component is a separate collective or original work to the maximum extent possible as defined by US copyright law. The inclusion in this collection of a component subject to a particular license does not limit your rights, or grant you rights in another component or in the collective work that supercede, the terms of the license of that other work.</p>"
        "<p><b>2. LIMITED REDISTRIBUTION RIGHTS</b><br/>"
        "This collective work may contain some components of binary code that are licensed under the GPLv2 license which limits your rights to redistribute the GPLv2 licensed components of this collective work to third parties, unless for a three year period you also distribute or formally offer to distribute to ANY party the source code of the component binary code that is licensed pursuant to the GPLv2 license. The Free Software Foundation of Boston, MA, US, as author of the GPLv2 license, claims to strictly enforce this requirement of the GPLv2 license for all parties with NO exceptions.</p>"
        "<p><b>3. RESPONSIBILITY TO COMPLY</b><br/>"
        "By using this package, you agree to comply with any applicable restrictions and obligations imposed by the GPLv2 license or any other applicable license, regulation, or law and to hold MEPIS LLC and its assigns harmless in case of your knowing or unknowing violation thereof.</p>"
        "<p><b>4. AVAILABILITY OF SOURCE CODE</b><br/>"
        "You may obtain a copy of GPL licensed source code from the source code package maintainer which is usually the next upstream copyright holder. In the case of MEPIS products, most GPL licensed binary code is redistributed without change.  In such cases, MEPIS is not a copyright holder in such code.  The maintained upstream sources of such code are available in the public Ubuntu and Debian software repositories. </p>"
        "<p><b>5. MEPIS OFFER OF SOURCE CODE</b><br/>"
        "MEPIS LLC hereby offers to provide, for three years from the original timestamp on any included GPLv2 licensed binary package, to any third party, a copy of the source code of the GPLv2 licensed binary code that is distributed with this collection, except where MEPIS LLC is prohibited from doing so by US law. For further details about this offer or to obtain said source code, visit http://www.mepis.org/source</p>"
        "<p><b>6. NO WARRANTY</b><br/>"
        "Except as specifically stated in a separate agreement or a license for a particular component, to the maximum extent permitted under applicable law, the collective work and the components are provided and licensed AS-IS without warranty of any kind, expressed or implied, including the implied warranties of merchantability, non-infringement or fitness for a particular purpose.</p>"
        "<p><b>7. ATTENTION US GOVERNMENT USERS</b><br/>"
        "The US Government's rights in this Software and accompanying documentation are only as set forth herein, in accordance with 48 CFR 227.7201 through 227.7202-4 and 48 CFR 2.101 and 12.212.</p>"
        "<p><b>8. CRYPTOGRAPHIC SOFTWARE RESTRICTIONS</b><br/>"
        "Some MEPIS packages are collections of software that contain unmodified publicly available Open Source encryption source code which, together with object code resulting from the compiling of publicly available source code, has been previously cleared for export from the United States to all persons, except Country Group E:1, under License Exception TSU pursuant to EAR Section 740.13(e).  Said source code and object code is publicly available at several web sites including ftp://mirror.mcs.anl.gov/pub/</p>"
        "<p>However, the knowing export or reexport of ANY cryptographic software and technology, including MEPIS related packages, from the US to nationals of Country Group E:1, currently Cuba, Iran, Libya, North Korea, Sudan, and Syria, is prohibited by US Law.</p>"
        "<p>In addition, at this time there MAY be export or import restrictions for products containing strong encryption (128 bit or greater) when sent to or from Armenia, Azerbaijan, Belarus, Burma, Republic of Congo, Egypt, France, Hong Kong, Israel, Kazakhstan, Liberia, Moldova, Nagorno-Karabakh, Pakistan, Philippines, Poland, Russia, Rwanda, Saudi Arabia, Sierra Leone, Somalia, Ukraine, Vietnam, or Yemen. </p>"
        "<p>These country lists are subject to change and other countries may impose restrictions on the import, export, and/or use of encryption software.</p>"
        "<p>This legal notice applies to cryptographic software only. More detailed information is available at http://www.bxa.doc.gov/</p>"
        "<p><b>9. PREVAILING LAW</b><br/>"
        "Whereas MEPIS LLC is a US based entity subject to US laws, by using this package, you agree that in cases of conflict of law US Copyright, Patent, Trademark, Contract, and Export Law shall prevail.</p>"
        "<p><b>10. IMPLICIT ACCEPTANCE</b><br/>"
        "By using and/or installing a MEPIS collective work, you accept and agree to comply with these Terms and Conditions.</p>");
      break;
  }
}

void MInstall::gotoPage(int next) {
  int curr = widgetStack->currentIndex();
  next = showPage(curr, next);

  // modify ui for standard cases
  if (next == 0) {
    // entering first page
    nextButton->setDefault(true);
    nextButton->setText(tr("Next >"));
    backButton->setEnabled(false);
  } else {
    // default
    backButton->setEnabled(true);
  }

  int c = widgetStack->count();
  if (next >= c-1) {
    // entering the last page
    nextButton->setText(tr("Finish"));
  } else {
    nextButton->setText(tr("Next >"));
  }
  if (next > c-1) {
    // finished
    stopInstall();
    gotoPage(0);
    return;
  }
  // display the next page
  widgetStack->setCurrentIndex(next);

  // anything to do after displaying the page
  pageDisplayed(next);
}

void MInstall::firstRefresh(QDialog *main) {
  mmn = main;
  refresh();
}

void MInstall::updatePartitionWidgets() 
{
   int numberPartitions = this->getPartitionNumber();

   if (numberPartitions > 0) {
      existing_partitionsButton->show();
   }
   else {
      existing_partitionsButton->hide();
      entireDiskButton->toggle(); 
   }
}

// widget being shown
void MInstall::refresh() {
  char line[130];

  this->updatePartitionWidgets();

//  system("umount -a 2>/dev/null");
  FILE *fp = popen("cat /proc/partitions | grep '[h,s,v].[a-z]$' | sort --key=4 2>/dev/null", "r");
  if (fp == NULL) {
    return;
  }
  diskCombo->clear();
  grubBootCombo->clear();
  char *ndev, *nsz;
  int i, nsize;
  while (fgets(line, sizeof line, fp) != NULL) {
    i = strlen(line);
    line[--i] = '\0';
    strtok(line, " \t");
    strtok(NULL, " \t");
    nsz = strtok(NULL, " \t");
    ndev = strtok(NULL, " \t");
    if (ndev != NULL && strlen(ndev) == 3) {
      nsize = atoi(nsz) / 1024;
      if (nsize > 1000) {
        sprintf(line, "%s", ndev);
        diskCombo->addItem(line);
        grubBootCombo->addItem(line);
      }
    }
  }
  pclose(fp);

  on_diskCombo_activated();

// locale
//  QString val = getCmdValue("cat /etc/default/locale", "LANG", " =", " ");
//  if (!val.isEmpty()) {
//    int lo = localeCombo->findText(val);
//    localeCombo->setCurrentIndex(lo);
//  }

  gotoPage(0);
}

/////////////////////////////////////////////////////////////////////////
// slots

void MInstall::on_passwordCheckBox_stateChanged(int state) {
  if (state == Qt::Unchecked) {
    // don't show
    userPasswordEdit->setEchoMode(QLineEdit::Password);
    userPasswordEdit2->setEchoMode(QLineEdit::Password);
    rootPasswordEdit->setEchoMode(QLineEdit::Password);
    rootPasswordEdit2->setEchoMode(QLineEdit::Password);
  } else {
    // show
    userPasswordEdit->setEchoMode(QLineEdit::Normal);
    userPasswordEdit2->setEchoMode(QLineEdit::Normal);
    rootPasswordEdit->setEchoMode(QLineEdit::Normal);
    rootPasswordEdit2->setEchoMode(QLineEdit::Normal);    
  }
}

void MInstall::on_nextButton_clicked() {
  int next = widgetStack->currentIndex() + 1;
  // make sure button is released
  nextButton->setDown(false);

  gotoPage(next);
}

void MInstall::on_backButton_clicked() {
  int curr = widgetStack->currentIndex();
  int next = curr - 1;

  gotoPage(next);
}

void MInstall::on_abortInstallButton_clicked() {
  procAbort();
  QApplication::beep();
}

void MInstall::on_qtpartedButton_clicked() {
  system("/sbin/swapoff -a 2>&1");
  system("/usr/sbin/gparted");
  system("/usr/sbin/buildfstab -r");
  system("/sbin/swapon -a 2>&1");
  this->updatePartitionWidgets();
  on_diskCombo_activated();
}

// disk selection changed, rebuild root
void MInstall::on_diskCombo_activated() {
  char line[130];
  QString drv = QString("/dev/%1").arg(diskCombo->currentText());

  rootCombo->clear();
  QString cmd = QString("/sbin/fdisk -l %1 | /bin/grep \"^/dev\"").arg(drv);
  FILE *fp = popen(cmd.toAscii(), "r");
  int rcount = 0;
  if (fp != NULL) {
    char *ndev, *nsz, *nsys, *nsys2;
    int nsize;
    int i;
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
      strtok(line, " /*+\t");
      ndev = strtok(NULL, " /*+\t");
      strtok(NULL, " *+\t");
      strtok(NULL, " *+\t");
      nsz = strtok(NULL, " *+\t");
      strtok(NULL, " *+\t");
      nsys = strtok(NULL, " *+\t");
      nsys2 = strtok(NULL, " *+\t");
      nsize = atoi(nsz);
      nsize = nsize / 1024;

      if ((nsize >= 1200) && (strncmp(nsys, "Linux", 5) == 0 || (strncmp(nsys, "FAT", 3) == 0) || (strncmp(nsys, "W95", 3) == 0) || (strncmp(nsys, "HPFS", 4) == 0))) {;
        sprintf(line, "%s - %dMB - %s", ndev, nsize, nsys);
        rootCombo->addItem(line);
        rcount++;
      }
    }
    pclose(fp);
  }
  if (rcount == 0) {
    rootCombo->addItem("none");
  }
  on_rootCombo_activated();
}

// root partition changed, rebuild swap
void MInstall::on_rootCombo_activated() {
  char line[130];
  QString drv = QString("/dev/%1").arg(diskCombo->currentText());

  swapCombo->clear();
  swapCombo->addItem("none - or existing");
  int rcount = 1;
  QString cmd = QString("/sbin/fdisk -l %1 | /bin/grep \"^/dev\"").arg(drv);
  FILE *fp = popen(cmd.toAscii(), "r");
  if (fp != NULL) {
    char *ndev, *nsz, *nsys, *nsys2;
    int nsize, i;
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
      strtok(line, " /*+\t");
      ndev = strtok(NULL, " /*+\t");
      strtok(NULL, " *+\t");
      strtok(NULL, " *+\t");
      nsz = strtok(NULL, " *+\t");
      strtok(NULL, " *+\t");
      nsys = strtok(NULL, " *+\t");
      nsys2 = strtok(NULL, " *+\t");
      nsize = atoi(nsz);
      nsize = nsize / 1024;
      if (nsys2 != NULL && strncmp(nsys2, "swap", 4) == 0) {
        sprintf(line, "%s - %dMB - %s", ndev, nsize, nsys2);
        swapCombo->addItem(line);
        rcount++;
      }  
    }
    pclose(fp);
  }
  on_swapCombo_activated();
}

// swap partition changed, rebuild home
void MInstall::on_swapCombo_activated() {;
  char line[130];
  QString drv = QString("/dev/%1").arg(diskCombo->currentText());

  homeCombo->clear();
  homeCombo->addItem("root");
  QString cmd = QString("/sbin/fdisk -l %1 | /bin/grep \"^/dev\"").arg(drv);
  FILE *fp = popen(cmd.toAscii(), "r");
  if (fp != NULL) {
    char *ndev, *nsz, *nsys, *nsys2;
    int nsize, i;
    while (fgets(line, sizeof line, fp) != NULL) {
      i = strlen(line);
      line[--i] = '\0';
      strtok(line, " /*+\t");
      ndev = strtok(NULL, " /*+\t");
      strtok(NULL, " *+\t");
      strtok(NULL, " *+\t");
      nsz = strtok(NULL, " *+\t");
      strtok(NULL, " *+\t");
      nsys = strtok(NULL, " *+\t");
      nsys2 = strtok(NULL, " *+\t");
      nsize = atoi(nsz);
      nsize = nsize / 1024;
      if (strcmp(ndev, rootCombo->currentText().section(' ', 0, 0).toAscii()) != 0 && 
            (nsize >= 100) && (strncmp(nsys, "Linux", 5) == 0) && (nsys2 == NULL)) {;
        sprintf(line, "%s - %dMB - %s", ndev, nsize, nsys);
        homeCombo->addItem(line);
      }
    }
    pclose(fp);
  }
}

void MInstall::on_rootTypeCombo_activated() {;
  if (rootTypeCombo->currentText().startsWith("ext")) {
    badblocksCheck->setEnabled(true);
  } else {
    badblocksCheck->setEnabled(false);
  }
    badblocksCheck->setChecked(false);
}

void MInstall::procAbort() {
  proc->terminate();
  QTimer::singleShot(5000, proc, SLOT(kill()));
}

bool MInstall::close() {
  if (proc->state() != QProcess::NotRunning) {
    int ans = QMessageBox::warning(0, QString::null,
        tr("Swift Linux is installing, are you \nsure you want to Close now?"),
        tr("Yes"), tr("No"));
    if (ans != 0) {
      return false;
    } else {
      procAbort();
    }
  }
//  system("umount -a 2>/dev/null");
  return QWidget::close();
}

/*
void MInstall::moreClicked(QListViewItem *item) {
  if (dansItem->isOn()) {
    squidItem->setOn(true);
  }

}
*/
/////////////////////////////////////////////////////////////////////////
// delete process events

void MInstall::delStart() {
  timer->start(20000);
  updateStatus(tr("Deleting old system"), 4);
}

void MInstall::delDone(int exitCode, QProcess::ExitStatus exitStatus) {
  if (exitStatus == QProcess::NormalExit) {
    copyLinux();
  } else {
    nextButton->setEnabled(true);
    unmountGoBack(tr("Failed to delete old Swift Linux on destination.\nReturning to Step 1."));
  }
}

void MInstall::delTime() {
  progressBar->setValue(progressBar->value() + 1);
}

/////////////////////////////////////////////////////////////////////////
// copy process events

void MInstall::copyStart() {
  timer->start(2000);
  updateStatus(tr("Copying new system"), 15);
}

void MInstall::copyDone(int exitCode, QProcess::ExitStatus exitStatus) {
  char line[130];
  char rootdev[20];
  char swapdev[20];
  char homedev[20];

  // get config
  strcpy(line, rootCombo->currentText().toAscii());
  char *tok = strtok(line, " -");
  sprintf(rootdev, "/dev/%s", tok);
  strcpy(line, swapCombo->currentText().toAscii());
  tok = strtok(line, " -");
  sprintf(swapdev, "/dev/%s", tok);
  strcpy(line, homeCombo->currentText().toAscii());
  tok = strtok(line, " -");
  sprintf(homedev, "/dev/%s", tok);

  timer->stop();

  if (exitStatus == QProcess::NormalExit) {
    updateStatus(tr("Fixing configuration"), 99);
    chmod("/mnt/antiX/var/tmp",01777);
    system("cd /mnt/antiX && ln -s var/tmp tmp");
    system("ls /lib64 && cd /mnt/antiX && ln -s lib lib64");

    FILE *fp = fopen("/mnt/antiX/etc/fstab", "w");
    if (fp != NULL) {
      fputs("# Pluggable devices are handled by uDev, they are not in fstab\n", fp);
      if (isRootFormatted) {
        if (isFormatExt3) {
          sprintf(line, "%s / ext3 defaults,noatime 1 1\n", rootdev);
        } else if (isFormatReiserfs) {
          sprintf(line, "%s / reiserfs defaults,noatime,notail 0 0\n", rootdev);
        } else {
          sprintf(line, "%s / ext4 defaults,noatime 1 1\n", rootdev);
        }
      } else {
        sprintf(line, "%s / auto defaults,noatime 1 1\n", rootdev);
      }
      fputs(line, fp);
      if (strcmp(swapdev, "/dev/none") != 0) {
        sprintf(line, "%s swap swap sw,pri=1 0 0\n", swapdev);
        fputs(line, fp);
      }
      fputs("proc /proc proc defaults 0 0\n", fp);
//      fputs("usbfs /proc/bus/usb usbfs devmode=0666 0 0\n", fp);
      fputs("devpts /dev/pts devpts mode=0622 0 0\n", fp);
//      fputs("sysfs /sys sysfs defaults 0 0\n", fp);
      if (strcmp(homedev, "/dev/root") != 0) {
        if (isHomeFormatted) {
          if (isFormatExt3) {
            sprintf(line, "%s /home ext3 defaults,noatime 1 2\n", homedev);
          } else if (isFormatReiserfs) {
            sprintf(line, "%s /home reiserfs defaults,noatime,notail 0 0\n", homedev);
          } else {
            sprintf(line, "%s /home ext4 defaults,noatime 1 2\n", homedev);
          }
        } else {
          sprintf(line, "%s /home auto defaults,noatime 1 2\n", homedev);
        }
        fputs(line, fp);
      }
      fclose(fp);
    }

    system("/bin/cp -fp /etc/X11/xorg.conf /mnt/antiX/etc/X11");
    system("/bin/cp -fp /etc/network/interfaces /mnt/antiX/etc/network");
    system("/bin/cp -fp /etc/rc.firewall /mnt/antiX/etc");
    system("/bin/cp -fp /etc/resolv.conf /mnt/antiX/etc");
    system("/bin/cp -fp /etc/powersave/* /mnt/antiX/etc/powersave");

    system("/bin/rm -f /mnt/antiX/etc/skel/Desktop/minstall.desktop");
    system("/bin/rm -f /mnt/antiX/root/Desktop/minstall.desktop");

    system("/bin/rm -f /mnt/antiX/home/*/.kde/share/config/kdesktoprc");
    system("/bin/rm -f /mnt/antiX/home/*/.kde/share/config/knetworkmanagerrc");

    progressBar->setValue(100);
    nextButton->setEnabled(true);
    QApplication::beep();
    setCursor(QCursor(Qt::ArrowCursor));
    on_nextButton_clicked();
  } else {
    nextButton->setEnabled(true);
    unmountGoBack(tr("Failed to write Swift Linux to destination.\nReturning to Step 1."));
  }
}

void MInstall::copyTime() {
  char line[130];
  char rootdev[20];
  strcpy(line, rootCombo->currentText().toAscii());
  char *tok = strtok(line, " -");
  sprintf(rootdev, "/dev/%s", tok);

  QString val = getCmdValue("df /mnt/antiX", rootdev, " ", "/");
  QRegExp sep("\\s+");
  QString s = val.section(sep, 2, 2);
  int i = s.toInt();
  val = getCmdValue("df /dev/loop0", "/dev/loop0", " ", "/");
  s = val.section(sep, 2, 2);
  int j = s.toInt()/27;
  i = i/j;
  if (i > 79) {
     i = 80;
  }
  progressBar->setValue(i + 15);

  switch (i) {
    case 1:
      tipsEdit->setText(tr("<p><b>Getting Help</b><br/>"
        "Basic information about Swift Linux is at http://www.swiftlinux.org "
        "Basic information about antiX Linux is at http://antix.mepis.com "
        "There are volunteers to help you at the antiX Forum, http://antix.freeforums.org</p>"
        "<p>If you ask for help, please remember to describe your problem and your computer "
        "in some detail. Usually statements like 'it didn't work' are not helpful.</p>"));
      break;
    
    case 15:
      tipsEdit->setText(tr("<p><b>Repairing Your Installation</b><br/>"
      "If Swift Linux stops working from the hard drive, sometimes it's possible to fix the problem by booting from CD and running one of the utilities in System Configuration or by using one of the regular Linux tools to repair the system.</p>"
      "<p>You can also use your Swift Linux CD to recover data from MS-Windows systems!</p>"));
      break;

    case 30:
      if (!getCmdValue("cat /etc/default/antiX","SERVER","="," ").contains("yes", Qt::CaseInsensitive)) {
      tipsEdit->setText(tr("<p><b>Support Swift Linux and antiX Linux</b><br/>"
      "antiX is supported by people like you. Some help others at the "
      "support forum - http://antix.freeforums.org, or translate help files into different "
      "languages, or make suggestions, write documentation, or help test new software.</p>"));
      }
      break;

    case 45:
      if (!getCmdValue("cat /etc/default/antiX","SERVER","="," ").contains("yes", Qt::CaseInsensitive)) {
      tipsEdit->setText(tr("<p><b>Adjusting Your Sound Mixer</b><br/>"
      "Swift Linux attempts to configure the sound mixer for you but sometimes it will be "
      "necessary for you to turn up volumes and unmute channels in the mixer "
      "in order to hear sound.</p> "
      "<p>The mixer shortcut is located in the tray. Click on it to open the mixer. </p>"));
      }
      break;

    case 60:
      tipsEdit->setText(tr("<p><b>Keep Your Copy of antiX Up-to-date</b><br/>"
        "For antiX information and updates please visit http://antix.freeforums.org. </p>"));
      break;

    default:
      break;
  } 
}

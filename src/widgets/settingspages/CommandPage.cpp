#include "CommandPage.hpp"

#include "Application.hpp"
#include "controllers/commands/Command.hpp"
#include "controllers/commands/CommandController.hpp"
#include "controllers/commands/CommandModel.hpp"
#include "singletons/Settings.hpp"
#include "util/CombinePath.hpp"
#include "util/LayoutCreator.hpp"
#include "util/Qt.hpp"
#include "util/StandardItemHelper.hpp"
#include "widgets/helper/EditableModelView.hpp"

#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QTextEdit>

// clang-format off
#define TEXT "{1} => first word &nbsp;&nbsp;&nbsp; {1+} => first word and after &nbsp;&nbsp;&nbsp; {{ => { &nbsp;&nbsp;&nbsp; <a href='https://chatterino.com/help/commands'>more info</a>"
// clang-format on

namespace chatterino {
namespace {
    QString c1settingsPath()
    {
        return combinePath(qgetenv("appdata"),
                           "Chatterino\\Custom\\Commands.txt");
    }
}  // namespace

CommandPage::CommandPage()
{
    auto *app = getApp();

    LayoutCreator<CommandPage> layoutCreator(this);
    auto layout = layoutCreator.setLayoutType<QVBoxLayout>();

    EditableModelView *view = layout
                                  .emplace<EditableModelView>(
                                      app->getCommands()->createModel(nullptr))
                                  .getElement();

    view->setTitles({"Trigger", "Command", "Show In\nMessage Menu"});
    view->getTableView()->horizontalHeader()->setSectionResizeMode(
        1, QHeaderView::Stretch);
    // We can safely ignore this signal connection since we own the view
    std::ignore = view->addButtonPressed.connect([] {
        getIApp()->getCommands()->items.append(
            Command{"/command", "I made a new command HeyGuys"});
    });

    // TODO: asyncronously check path
    if (QFile(c1settingsPath()).exists())
    {
        auto *button = new QPushButton("Import commands from Chatterino 1");
        view->addCustomButton(button);

        QObject::connect(button, &QPushButton::clicked, this, [] {
            QFile c1settings = c1settingsPath();
            c1settings.open(QIODevice::ReadOnly);
            for (auto line :
                 QString(c1settings.readAll())
                     .split(QRegularExpression("[\r\n]"), Qt::SkipEmptyParts))
            {
                if (int index = line.indexOf(' '); index != -1)
                {
                    getIApp()->getCommands()->items.insert(
                        Command(line.mid(0, index), line.mid(index + 1)));
                }
            }
        });
    }

    layout.append(
        this->createCheckBox("Also match the trigger at the end of the message",
                             getSettings()->allowCommandsAtEnd));

    QLabel *text = layout.emplace<QLabel>(TEXT).getElement();
    text->setWordWrap(true);
    text->setStyleSheet("color: #bbb");
    text->setOpenExternalLinks(true);

    // ---- end of layout
    this->commandsEditTimer_.setSingleShot(true);
}

}  // namespace chatterino

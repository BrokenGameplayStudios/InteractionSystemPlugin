#include "InteractionPromptWidget.h"
#include "InteractionNotificationSubsystem.h"

void UInteractionPromptWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (UInteractionNotificationSubsystem* Subsystem = PC->GetLocalPlayer()->GetSubsystem<UInteractionNotificationSubsystem>())
        {
            Subsystem->OnInteractionPromptUpdated.AddDynamic(this, &UInteractionPromptWidget::OnSubsystemPromptUpdated);
        }
    }
}

void UInteractionPromptWidget::NativeDestruct()
{
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (UInteractionNotificationSubsystem* Subsystem = PC->GetLocalPlayer()->GetSubsystem<UInteractionNotificationSubsystem>())
        {
            Subsystem->OnInteractionPromptUpdated.RemoveDynamic(this, &UInteractionPromptWidget::OnSubsystemPromptUpdated);
        }
    }
    Super::NativeDestruct();
}

void UInteractionPromptWidget::OnSubsystemPromptUpdated(FInteractionPromptData Data)
{
    CurrentData = Data;
    BP_OnPromptUpdated(Data);
    UpdateKeyIcon(Data.InputActionName);
}

void UInteractionPromptWidget::UpdateKeyIcon(FName ActionName)
{
    // Leave empty — we do the real key display in your Blueprint child widget
}
#include "InteractionNotificationSubsystem.h"

void UInteractionNotificationSubsystem::UpdateInteractionPrompt(const FInteractionPromptData& Data)
{
    OnInteractionPromptUpdated.Broadcast(Data);
}

void UInteractionNotificationSubsystem::ClearInteractionPrompt()
{
    FInteractionPromptData EmptyData;
    OnInteractionPromptUpdated.Broadcast(EmptyData);
}

void UInteractionNotificationSubsystem::ShowTemporaryNotification(const FText& Message, float Duration, bool bIsWarning)
{
    FInteractionPromptData Data;
    Data.PrimaryText = Message;
    Data.bIsWarning = bIsWarning;
    Data.AutoHideTime = Duration;
    UpdateInteractionPrompt(Data);
}
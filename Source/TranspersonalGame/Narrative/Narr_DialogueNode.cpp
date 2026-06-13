#include "Narr_DialogueNode.h"
#include "Engine/Engine.h"

UNarr_DialogueNode::UNarr_DialogueNode()
{
    // Initialize with default values
}

bool UNarr_DialogueNode::CanExecute(const TMap<FString, int32>& GameState) const
{
    return CheckConditions(DialogueData.TriggerConditions, GameState);
}

TArray<FNarr_DialogueChoice> UNarr_DialogueNode::GetValidChoices(const TMap<FString, int32>& GameState) const
{
    TArray<FNarr_DialogueChoice> ValidChoices;
    
    for (const FNarr_DialogueChoice& Choice : DialogueData.Choices)
    {
        if (CheckConditions(Choice.RequiredConditions, GameState))
        {
            ValidChoices.Add(Choice);
        }
    }
    
    return ValidChoices;
}

void UNarr_DialogueNode::ExecuteCompletionActions()
{
    for (const FString& Action : DialogueData.CompletionActions)
    {
        UE_LOG(LogTemp, Warning, TEXT("Executing dialogue action: %s"), *Action);
        
        // Parse and execute actions
        if (Action.StartsWith(TEXT("set_progress:")))
        {
            // Format: set_progress:key:value
            TArray<FString> Parts;
            Action.ParseIntoArray(Parts, TEXT(":"), true);
            
            if (Parts.Num() == 3)
            {
                FString Key = Parts[1];
                int32 Value = FCString::Atoi(*Parts[2]);
                
                UE_LOG(LogTemp, Log, TEXT("Setting progress %s to %d"), *Key, Value);
            }
        }
        else if (Action.StartsWith(TEXT("trigger_event:")))
        {
            // Format: trigger_event:event_id
            FString EventID = Action.RightChop(14); // Remove "trigger_event:"
            UE_LOG(LogTemp, Log, TEXT("Triggering event: %s"), *EventID);
        }
        else if (Action.StartsWith(TEXT("give_item:")))
        {
            // Format: give_item:item_id:quantity
            TArray<FString> Parts;
            Action.ParseIntoArray(Parts, TEXT(":"), true);
            
            if (Parts.Num() >= 2)
            {
                FString ItemID = Parts[1];
                int32 Quantity = Parts.Num() > 2 ? FCString::Atoi(*Parts[2]) : 1;
                
                UE_LOG(LogTemp, Log, TEXT("Giving item %s x%d"), *ItemID, Quantity);
            }
        }
    }
}

FString UNarr_DialogueNode::GetSpeakerName() const
{
    switch (DialogueData.Speaker)
    {
        case ENarr_SpeakerType::Player:
            return TEXT("You");
        case ENarr_SpeakerType::TribalElder:
            return TEXT("Elder Krog");
        case ENarr_SpeakerType::Hunter:
            return TEXT("Hunter");
        case ENarr_SpeakerType::Survivor:
            return TEXT("Survivor");
        case ENarr_SpeakerType::Narrator:
            return TEXT("Narrator");
        case ENarr_SpeakerType::Warning:
            return TEXT("Warning");
        default:
            return TEXT("Unknown");
    }
}

bool UNarr_DialogueNode::CheckConditions(const TArray<FString>& Conditions, const TMap<FString, int32>& GameState) const
{
    for (const FString& Condition : Conditions)
    {
        if (!ParseCondition(Condition, GameState).Equals(TEXT("true")))
        {
            return false;
        }
    }
    return true;
}

FString UNarr_DialogueNode::ParseCondition(const FString& Condition, const TMap<FString, int32>& GameState) const
{
    // Parse conditions like "health > 50", "has_item:spear", "story_phase == 2"
    
    if (Condition.Contains(TEXT(">")))
    {
        TArray<FString> Parts;
        Condition.ParseIntoArray(Parts, TEXT(">"), true);
        
        if (Parts.Num() == 2)
        {
            FString Key = Parts[0].TrimStartAndEnd();
            int32 Value = FCString::Atoi(*Parts[1].TrimStartAndEnd());
            
            if (const int32* StateValue = GameState.Find(Key))
            {
                return (*StateValue > Value) ? TEXT("true") : TEXT("false");
            }
        }
    }
    else if (Condition.Contains(TEXT("<")))
    {
        TArray<FString> Parts;
        Condition.ParseIntoArray(Parts, TEXT("<"), true);
        
        if (Parts.Num() == 2)
        {
            FString Key = Parts[0].TrimStartAndEnd();
            int32 Value = FCString::Atoi(*Parts[1].TrimStartAndEnd());
            
            if (const int32* StateValue = GameState.Find(Key))
            {
                return (*StateValue < Value) ? TEXT("true") : TEXT("false");
            }
        }
    }
    else if (Condition.Contains(TEXT("==")))
    {
        TArray<FString> Parts;
        Condition.ParseIntoArray(Parts, TEXT("=="), true);
        
        if (Parts.Num() == 2)
        {
            FString Key = Parts[0].TrimStartAndEnd();
            int32 Value = FCString::Atoi(*Parts[1].TrimStartAndEnd());
            
            if (const int32* StateValue = GameState.Find(Key))
            {
                return (*StateValue == Value) ? TEXT("true") : TEXT("false");
            }
        }
    }
    else if (Condition.StartsWith(TEXT("has_item:")))
    {
        FString ItemKey = Condition.RightChop(9); // Remove "has_item:"
        if (const int32* ItemCount = GameState.Find(ItemKey))
        {
            return (*ItemCount > 0) ? TEXT("true") : TEXT("false");
        }
    }
    else if (Condition.StartsWith(TEXT("not_has_item:")))
    {
        FString ItemKey = Condition.RightChop(13); // Remove "not_has_item:"
        if (const int32* ItemCount = GameState.Find(ItemKey))
        {
            return (*ItemCount <= 0) ? TEXT("true") : TEXT("false");
        }
        return TEXT("true"); // If item not in state, assume player doesn't have it
    }
    
    // Default: check if key exists and is > 0
    if (const int32* StateValue = GameState.Find(Condition))
    {
        return (*StateValue > 0) ? TEXT("true") : TEXT("false");
    }
    
    return TEXT("false");
}
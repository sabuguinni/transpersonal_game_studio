#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "../SharedTypes.h"
#include "SurvivalNarrativeLibrary.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_SurvivalDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Dialogue")
    FString DialogueID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Dialogue")
    FString SpeakerName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Dialogue")
    ENarr_SurvivalContext Context;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Dialogue")
    float Priority;

    FNarr_SurvivalDialogue()
    {
        DialogueID = TEXT("");
        SpeakerName = TEXT("");
        DialogueText = TEXT("");
        Context = ENarr_SurvivalContext::General;
        Priority = 1.0f;
    }
};

UCLASS()
class TRANSPERSONALGAME_API USurvivalNarrativeLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    static FString GetSurvivalDialogue(ENarr_SurvivalContext Context, float PlayerHealth, float PlayerHunger);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    static FString GetDinosaurEncounterDialogue(ENarr_DinosaurType DinosaurType, float Distance);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    static FString GetEnvironmentDialogue(ENarr_BiomeType BiomeType, ENarr_TimeOfDay TimeOfDay);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    static FString GetCraftingDialogue(ENarr_CraftingAction Action, bool bSuccess);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    static TArray<FNarr_SurvivalDialogue> GetContextualDialogues(ENarr_SurvivalContext Context);

    UFUNCTION(BlueprintCallable, Category = "Survival Narrative")
    static FString FormatSurvivalStatus(float Health, float Hunger, float Thirst, float Stamina);

private:
    static TMap<ENarr_SurvivalContext, TArray<FString>> SurvivalDialogueDatabase;
    static TMap<ENarr_DinosaurType, TArray<FString>> DinosaurDialogueDatabase;
    static TMap<ENarr_BiomeType, TArray<FString>> EnvironmentDialogueDatabase;
    
    static void InitializeDialogueDatabases();
    static FString SelectRandomDialogue(const TArray<FString>& DialogueOptions);
};
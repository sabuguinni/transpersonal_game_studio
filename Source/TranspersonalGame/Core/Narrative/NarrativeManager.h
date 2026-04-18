#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Engine/DataTable.h"
#include "../SharedTypes.h"
#include "NarrativeManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_StoryEvent
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FString EventName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    FText EventDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    bool bIsCompleted;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> Prerequisites;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Story")
    TArray<FString> UnlockedEvents;

    FNarr_StoryEvent()
        : bIsCompleted(false)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_CharacterProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FText BackgroundStory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> PersonalityTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    int32 TrustLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> KnownStoryEvents;

    FNarr_CharacterProfile()
        : TrustLevel(0)
    {}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNarr_WorldLore
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FString LoreID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FString LoreCategory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FText LoreTitle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    FText LoreContent;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lore")
    bool bIsDiscovered;

    FNarr_WorldLore()
        : bIsDiscovered(false)
    {}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStoryEventTriggered, const FString&, EventID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCharacterTrustChanged, const FString&, CharacterID, int32, NewTrustLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoreDiscovered, const FString&, LoreID);

UCLASS()
class TRANSPERSONALGAME_API UNarrativeManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    // Story Events
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void TriggerStoryEvent(const FString& EventID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsStoryEventCompleted(const FString& EventID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FString> GetAvailableStoryEvents() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddStoryEvent(const FNarr_StoryEvent& NewEvent);

    // Character Management
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void ModifyCharacterTrust(const FString& CharacterID, int32 TrustChange);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    int32 GetCharacterTrust(const FString& CharacterID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddCharacterProfile(const FNarr_CharacterProfile& NewProfile);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    FNarr_CharacterProfile GetCharacterProfile(const FString& CharacterID) const;

    // World Lore
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void DiscoverLore(const FString& LoreID);

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    bool IsLoreDiscovered(const FString& LoreID) const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    TArray<FNarr_WorldLore> GetDiscoveredLore() const;

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void AddWorldLore(const FNarr_WorldLore& NewLore);

    // Save/Load
    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void SaveNarrativeState();

    UFUNCTION(BlueprintCallable, Category = "Narrative")
    void LoadNarrativeState();

    // Events
    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnStoryEventTriggered OnStoryEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnCharacterTrustChanged OnCharacterTrustChanged;

    UPROPERTY(BlueprintAssignable, Category = "Narrative Events")
    FOnLoreDiscovered OnLoreDiscovered;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_StoryEvent> StoryEvents;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_CharacterProfile> CharacterProfiles;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Narrative", meta = (AllowPrivateAccess = "true"))
    TArray<FNarr_WorldLore> WorldLore;

private:
    void InitializeDefaultContent();
    bool CheckEventPrerequisites(const FNarr_StoryEvent& Event) const;
    void UnlockDependentEvents(const FString& CompletedEventID);
};

#include "NarrativeManager.generated.h"
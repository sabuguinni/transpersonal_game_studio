#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/Engine.h"
#include "SharedTypes.h"
#include "Quest_NPCManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCDialogue
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString DialogueText;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    FString AudioURL;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
    bool bIsQuestDialogue;

    FQuest_NPCDialogue()
    {
        DialogueText = TEXT("");
        AudioURL = TEXT("");
        Duration = 0.0f;
        bIsQuestDialogue = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FQuest_NPCProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENPCRole Role;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FQuest_NPCDialogue> AvailableDialogues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FString> QuestIDs;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FVector SpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    bool bIsActive;

    FQuest_NPCProfile()
    {
        NPCName = TEXT("Unnamed NPC");
        Role = ENPCRole::Neutral;
        SpawnLocation = FVector::ZeroVector;
        bIsActive = true;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AQuest_NPCActor : public AActor
{
    GENERATED_BODY()

public:
    AQuest_NPCActor();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* NPCMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* InteractionSphere;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    FQuest_NPCProfile NPCProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Data")
    int32 CurrentDialogueIndex;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void StartDialogue();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    FQuest_NPCDialogue GetCurrentDialogue();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    void AdvanceDialogue();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    bool HasQuestAvailable();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC")
    TArray<FString> GetAvailableQuests();

    UFUNCTION()
    void OnInteractionSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UQuest_NPCManager : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

protected:
    UPROPERTY()
    TArray<FQuest_NPCProfile> RegisteredNPCs;

    UPROPERTY()
    TArray<AQuest_NPCActor*> ActiveNPCActors;

public:
    UFUNCTION(BlueprintCallable, Category = "Quest NPC Manager")
    void RegisterNPC(const FQuest_NPCProfile& NPCProfile);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Manager")
    void SpawnNPC(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Manager")
    void SpawnAllNPCs();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Manager")
    AQuest_NPCActor* FindNPCByName(const FString& NPCName);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Manager")
    TArray<AQuest_NPCActor*> GetNPCsByRole(ENPCRole Role);

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Manager")
    void SetupDefaultNPCs();

    UFUNCTION(BlueprintCallable, Category = "Quest NPC Manager")
    int32 GetActiveNPCCount() const { return ActiveNPCActors.Num(); }
};
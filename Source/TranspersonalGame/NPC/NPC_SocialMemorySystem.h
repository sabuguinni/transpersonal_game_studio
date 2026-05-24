#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "SharedTypes.h"
#include "NPC_SocialMemorySystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    AActor* TargetActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float RelationshipValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    float LastInteractionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    FVector LastSeenLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    ENPC_InteractionType LastInteractionType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    bool bIsHostile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    bool bIsTrusted;

    FNPC_SocialMemory()
    {
        TargetActor = nullptr;
        RelationshipValue = 0.0f;
        LastInteractionTime = 0.0f;
        LastSeenLocation = FVector::ZeroVector;
        LastInteractionType = ENPC_InteractionType::Neutral;
        bIsHostile = false;
        bIsTrusted = false;
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_TerritorialMemory
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FVector TerritoryCenter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float TerritoryRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    TArray<AActor*> IntrudersInTerritory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    float LastIntrusionTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    bool bIsDefending;

    FNPC_TerritorialMemory()
    {
        TerritoryCenter = FVector::ZeroVector;
        TerritoryRadius = 2000.0f;
        LastIntrusionTime = 0.0f;
        bIsDefending = false;
    }
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPC_SocialMemorySystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPC_SocialMemorySystem();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Social Memory Management
    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void RecordInteraction(AActor* TargetActor, ENPC_InteractionType InteractionType, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    FNPC_SocialMemory GetSocialMemory(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    void UpdateRelationship(AActor* TargetActor, float RelationshipChange);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    bool IsHostile(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    bool IsTrusted(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Social Memory")
    TArray<AActor*> GetKnownActors();

    // Territorial Memory Management
    UFUNCTION(BlueprintCallable, Category = "Territory")
    void SetTerritory(FVector Center, float Radius);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    bool IsInTerritory(AActor* Actor);

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void CheckTerritorialIntrusion();

    UFUNCTION(BlueprintCallable, Category = "Territory")
    void DefendTerritory(bool bStartDefending);

    // Memory Decay
    UFUNCTION(BlueprintCallable, Category = "Memory")
    void DecayMemories(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ForgetActor(AActor* TargetActor);

    UFUNCTION(BlueprintCallable, Category = "Memory")
    void ClearAllMemories();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Social Memory")
    TArray<FNPC_SocialMemory> SocialMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Territory")
    FNPC_TerritorialMemory TerritorialMemory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryDecayRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MaxMemoryDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxSocialMemories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float SightRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Detection")
    float HearingRange;

private:
    FNPC_SocialMemory* FindSocialMemory(AActor* TargetActor);
    void CleanupOldMemories();
    void UpdateLastSeenLocation(AActor* TargetActor);
};
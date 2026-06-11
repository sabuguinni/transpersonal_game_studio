#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/Engine.h"
#include "TranspersonalGame/Core/SharedTypes.h"
#include "NPCMemorySystem.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_MemoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    FVector Location = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    AActor* Actor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    ENPC_MemoryType MemoryType = ENPC_MemoryType::PlayerSighting;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float Timestamp = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float Importance = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float DecayRate = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    bool bIsActive = true;
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FNPC_SocialRelation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    AActor* TargetActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    float RelationshipValue = 0.0f; // -1.0 (hostile) to 1.0 (friendly)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    float TrustLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    float FearLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    int32 InteractionCount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    float LastInteractionTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Social")
    TArray<FNPC_MemoryEntry> SharedMemories;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPC_MemorySystem : public UObject
{
    GENERATED_BODY()

public:
    UNPC_MemorySystem();

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void InitializeMemorySystem(class ANPC_BaseCharacter* NPCOwner);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void UpdateMemorySystem(float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void AddMemory(const FVector& Location, AActor* Actor, ENPC_MemoryType MemoryType, float Importance = 1.0f, const FString& Description = TEXT(""));

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void RemoveMemory(int32 MemoryIndex);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesByType(ENPC_MemoryType MemoryType) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> GetMemoriesNearLocation(const FVector& Location, float Radius) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_MemoryEntry GetMostRecentMemory(ENPC_MemoryType MemoryType) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool HasMemoryOfActor(AActor* Actor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void UpdateSocialRelation(AActor* TargetActor, float RelationshipDelta, float TrustDelta = 0.0f, float FearDelta = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    FNPC_SocialRelation GetSocialRelation(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool IsHostileTowards(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    bool IsFriendlyTowards(AActor* TargetActor) const;

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ShareMemoryWithNPC(UNPC_MemorySystem* OtherNPC, const FNPC_MemoryEntry& Memory);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    void ForgetOldMemories(float MaxAge);

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    int32 GetMemoryCount() const { return Memories.Num(); }

    UFUNCTION(BlueprintCallable, Category = "NPC Memory")
    float GetMemoryImportance(const FNPC_MemoryEntry& Memory) const;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory")
    TArray<FNPC_MemoryEntry> Memories;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory")
    TArray<FNPC_SocialRelation> SocialRelations;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "NPC Memory")
    class ANPC_BaseCharacter* OwnerNPC;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    int32 MaxMemories = 50;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryDecayRate = 0.05f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float SocialDecayRate = 0.02f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC Memory")
    float MemoryShareRadius = 500.0f;

private:
    void DecayMemories(float DeltaTime);
    void DecaySocialRelations(float DeltaTime);
    void PruneOldMemories();
    FNPC_SocialRelation* FindSocialRelation(AActor* TargetActor);
    float GetCurrentGameTime() const;
};
// Copyright Transpersonal Game Studio. All Rights Reserved.
// NPC Memory System V44 - Advanced episodic and semantic memory for believable NPCs

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/ActorComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "NPCMemorySystemV44.generated.h"

UENUM(BlueprintType)
enum class EMemoryType : uint8
{
    Episodic            UMETA(DisplayName = "Episodic"),        // Specific events and experiences
    Semantic            UMETA(DisplayName = "Semantic"),       // General knowledge and facts
    Procedural          UMETA(DisplayName = "Procedural"),     // Skills and habits
    Emotional           UMETA(DisplayName = "Emotional"),      // Emotionally charged memories
    Spatial             UMETA(DisplayName = "Spatial"),        // Location-based memories
    Social              UMETA(DisplayName = "Social"),         // Relationship and interaction memories
    Sensory             UMETA(DisplayName = "Sensory")         // Sight, sound, smell memories
};

UENUM(BlueprintType)
enum class EMemoryImportance : uint8
{
    Trivial             UMETA(DisplayName = "Trivial"),
    Low                 UMETA(DisplayName = "Low"),
    Medium              UMETA(DisplayName = "Medium"),
    High                UMETA(DisplayName = "High"),
    Critical            UMETA(DisplayName = "Critical"),
    Traumatic           UMETA(DisplayName = "Traumatic")       // Never forgotten
};

USTRUCT(BlueprintType)
struct FMemoryFragment
{
    GENERATED_BODY()

    // Core memory data
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString MemoryID = "";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMemoryType MemoryType = EMemoryType::Episodic;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EMemoryImportance Importance = EMemoryImportance::Medium;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime Timestamp;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector Location = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Description = "";
    
    // Associated actors and objects
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> InvolvedActors;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* PrimaryActor = nullptr;
    
    // Emotional context
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "-1.0", ClampMax = "1.0"))
    float EmotionalValence = 0.0f; // -1.0 = very negative, 1.0 = very positive
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float EmotionalIntensity = 0.5f; // How emotionally intense the memory is
    
    // Memory strength and decay
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Strength = 1.0f; // How vivid/accessible the memory is
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float DecayRate = 0.01f; // How fast this memory fades
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AccessCount = 0; // How many times this memory has been recalled
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime LastAccessed;
    
    // Contextual information
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TMap<FString, FString> ContextualData; // Additional key-value data
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> AssociatedTags; // Tags for memory retrieval
    
    // Sensory details
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString VisualDetails = "";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString AudioDetails = "";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString SmellDetails = "";
    
    // Memory connections
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RelatedMemoryIDs; // Connected memories
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Coherence = 1.0f; // How well-formed and consistent the memory is

    FMemoryFragment()
    {
        Timestamp = FDateTime::Now();
        LastAccessed = FDateTime::Now();
        MemoryID = FGuid::NewGuid().ToString();
    }
};

USTRUCT(BlueprintType)
struct FMemoryCluster
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClusterID = "";
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ClusterTheme = ""; // What this cluster represents
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> MemoryIDs; // Memories in this cluster
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector CentralLocation = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime FormationTime;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float Cohesion = 0.5f; // How strongly connected the memories are
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    AActor* CentralActor = nullptr; // Main actor this cluster relates to

    FMemoryCluster()
    {
        FormationTime = FDateTime::Now();
        ClusterID = FGuid::NewGuid().ToString();
    }
};

USTRUCT(BlueprintType)
struct FMemoryQuery
{
    GENERATED_BODY()

    // Query parameters
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EMemoryType> MemoryTypes;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<AActor*> InvolvedActors;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector LocationCenter = FVector::ZeroVector;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LocationRadius = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime TimeFrom = FDateTime::MinValue();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FDateTime TimeTo = FDateTime::MaxValue();
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> RequiredTags;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FString> ExcludedTags;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinEmotionalIntensity = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MinStrength = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxResults = 10;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSortByRecency = true;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSortByImportance = false;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bSortByEmotionalIntensity = false;
};

/**
 * Advanced NPC Memory System V44
 * Implements episodic and semantic memory with emotional weighting, decay, and clustering
 * Allows NPCs to remember past events, learn from experiences, and form coherent narratives
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UNPCMemorySystemV44 : public UActorComponent
{
    GENERATED_BODY()

public:
    UNPCMemorySystemV44();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
    // === MEMORY STORAGE ===
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Storage")
    TMap<FString, FMemoryFragment> AllMemories;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Storage")
    TArray<FMemoryCluster> MemoryClusters;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Storage")
    TMap<EMemoryType, TArray<FString>> MemoriesByType;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Storage")
    TMap<AActor*, TArray<FString>> MemoriesByActor;
    
    // === CONFIGURATION ===
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Config")
    int32 MaxTotalMemories = 200;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Config")
    int32 MaxMemoriesPerType = 50;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Config")
    float BaseDecayRate = 0.001f; // Base memory decay per second
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Config")
    float EmotionalDecayResistance = 0.5f; // How much emotion slows decay
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Config")
    float ImportanceDecayResistance = 0.3f; // How much importance slows decay
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Config")
    float AccessDecayResistance = 0.2f; // How much recent access slows decay
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Config")
    float ClusteringThreshold = 0.7f; // Similarity threshold for clustering
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Config")
    float ConsolidationInterval = 60.0f; // How often to consolidate memories (seconds)
    
    // === MEMORY CREATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Memory Creation")
    FString CreateMemory(
        EMemoryType MemoryType,
        const FString& Description,
        FVector Location = FVector::ZeroVector,
        AActor* PrimaryActor = nullptr,
        const TArray<AActor*>& InvolvedActors = TArray<AActor*>(),
        float EmotionalValence = 0.0f,
        float EmotionalIntensity = 0.5f,
        EMemoryImportance Importance = EMemoryImportance::Medium,
        const TArray<FString>& Tags = TArray<FString>()
    );
    
    UFUNCTION(BlueprintCallable, Category = "Memory Creation")
    FString CreateEpisodicMemory(
        const FString& EventDescription,
        AActor* PrimaryActor,
        FVector Location,
        float EmotionalValence = 0.0f,
        float EmotionalIntensity = 0.5f,
        EMemoryImportance Importance = EMemoryImportance::Medium
    );
    
    UFUNCTION(BlueprintCallable, Category = "Memory Creation")
    FString CreateSocialMemory(
        AActor* OtherActor,
        const FString& InteractionDescription,
        float EmotionalValence = 0.0f,
        float EmotionalIntensity = 0.5f,
        FVector Location = FVector::ZeroVector
    );
    
    UFUNCTION(BlueprintCallable, Category = "Memory Creation")
    FString CreateSpatialMemory(
        FVector Location,
        const FString& LocationDescription,
        const FString& LocationType = "Unknown",
        EMemoryImportance Importance = EMemoryImportance::Medium
    );
    
    // === MEMORY RETRIEVAL ===
    
    UFUNCTION(BlueprintCallable, Category = "Memory Retrieval")
    TArray<FMemoryFragment> QueryMemories(const FMemoryQuery& Query);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Retrieval")
    TArray<FMemoryFragment> GetMemoriesOfActor(AActor* Actor, int32 MaxResults = 10);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Retrieval")
    TArray<FMemoryFragment> GetMemoriesAtLocation(FVector Location, float Radius = 500.0f, int32 MaxResults = 10);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Retrieval")
    TArray<FMemoryFragment> GetMemoriesWithTag(const FString& Tag, int32 MaxResults = 10);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Retrieval")
    TArray<FMemoryFragment> GetRecentMemories(float HoursBack = 24.0f, int32 MaxResults = 10);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Retrieval")
    TArray<FMemoryFragment> GetEmotionalMemories(float MinIntensity = 0.7f, int32 MaxResults = 10);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Retrieval")
    FMemoryFragment GetMemoryByID(const FString& MemoryID);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Retrieval")
    TArray<FMemoryFragment> GetRelatedMemories(const FString& MemoryID, int32 MaxResults = 5);
    
    // === MEMORY MODIFICATION ===
    
    UFUNCTION(BlueprintCallable, Category = "Memory Modification")
    void AccessMemory(const FString& MemoryID);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Modification")
    void ReinforceMemory(const FString& MemoryID, float StrengthBonus = 0.1f);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Modification")
    void WeakenMemory(const FString& MemoryID, float StrengthPenalty = 0.1f);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Modification")
    void AddMemoryTag(const FString& MemoryID, const FString& Tag);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Modification")
    void RemoveMemoryTag(const FString& MemoryID, const FString& Tag);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Modification")
    void LinkMemories(const FString& MemoryID1, const FString& MemoryID2);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Modification")
    void UpdateMemoryEmotionalContext(const FString& MemoryID, float NewValence, float NewIntensity);
    
    // === MEMORY MANAGEMENT ===
    
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ForgetMemory(const FString& MemoryID);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ForgetActor(AActor* Actor);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ForgetLocation(FVector Location, float Radius = 100.0f);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void ConsolidateMemories();
    
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void PurgeWeakMemories(float MinStrength = 0.1f);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    int32 GetMemoryCount() const { return AllMemories.Num(); }
    
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    float GetMemoryLoad() const { return (float)AllMemories.Num() / (float)MaxTotalMemories; }
    
    // === MEMORY CLUSTERING ===
    
    UFUNCTION(BlueprintCallable, Category = "Memory Clustering")
    void CreateMemoryCluster(const TArray<FString>& MemoryIDs, const FString& Theme);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Clustering")
    TArray<FMemoryCluster> GetMemoryClustersForActor(AActor* Actor);
    
    UFUNCTION(BlueprintCallable, Category = "Memory Clustering")
    TArray<FMemoryCluster> GetMemoryClustersAtLocation(FVector Location, float Radius = 1000.0f);
    
    // === MEMORY ANALYSIS ===
    
    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    float GetActorFamiliarity(AActor* Actor) const;
    
    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    float GetLocationFamiliarity(FVector Location, float Radius = 500.0f) const;
    
    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    float GetAverageEmotionalValenceForActor(AActor* Actor) const;
    
    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    TArray<FString> GetMostAccessedMemories(int32 Count = 5) const;
    
    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    TArray<AActor*> GetMostRememberedActors(int32 Count = 5) const;
    
    UFUNCTION(BlueprintCallable, Category = "Memory Analysis")
    TArray<FVector> GetMostVisitedLocations(int32 Count = 5) const;

protected:
    // Internal processing functions
    void UpdateMemoryDecay(float DeltaTime);
    void ProcessMemoryClustering();
    void OptimizeMemoryStorage();
    
    // Helper functions
    float CalculateMemorySimilarity(const FMemoryFragment& Memory1, const FMemoryFragment& Memory2) const;
    float CalculateDecayRate(const FMemoryFragment& Memory) const;
    void UpdateMemoryIndices(const FString& MemoryID);
    void RemoveMemoryFromIndices(const FString& MemoryID);
    TArray<FMemoryFragment> SortMemoriesByRelevance(const TArray<FMemoryFragment>& Memories, const FMemoryQuery& Query) const;
    
    // Timer handles
    FTimerHandle ConsolidationTimer;
    FTimerHandle ClusteringTimer;
    
    // Performance tracking
    float LastDecayUpdate = 0.0f;
    float DecayUpdateInterval = 1.0f; // Update decay once per second
    
    // Memory statistics
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Statistics")
    int32 TotalMemoriesCreated = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Statistics")
    int32 TotalMemoriesForgotten = 0;
    
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory Statistics")
    float AverageMemoryStrength = 1.0f;
};
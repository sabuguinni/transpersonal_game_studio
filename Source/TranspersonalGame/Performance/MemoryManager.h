// MemoryManager.h
// Sistema de gestão inteligente de memória para otimização de performance

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/PlatformMemory.h"
#include "Containers/Map.h"
#include "Containers/Queue.h"
#include "UObject/WeakObjectPtr.h"
#include "MemoryManager.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalMemory, Log, All);

// Estrutura para tracking de objetos em memória
USTRUCT()
struct TRANSPERSONALGAME_API FMemoryObjectInfo
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<UObject> Object;

    UPROPERTY()
    FString ObjectName;

    UPROPERTY()
    FString ClassName;

    UPROPERTY()
    int64 EstimatedSize;

    UPROPERTY()
    float LastAccessTime;

    UPROPERTY()
    int32 AccessCount;

    UPROPERTY()
    bool bIsCritical;

    FMemoryObjectInfo()
        : EstimatedSize(0)
        , LastAccessTime(0.0f)
        , AccessCount(0)
        , bIsCritical(false)
    {}

    FMemoryObjectInfo(UObject* InObject, int64 InSize, bool bInIsCritical = false)
        : Object(InObject)
        , EstimatedSize(InSize)
        , LastAccessTime(FPlatformTime::Seconds())
        , AccessCount(1)
        , bIsCritical(bInIsCritical)
    {
        if (InObject)
        {
            ObjectName = InObject->GetName();
            ClassName = InObject->GetClass()->GetName();
        }
    }
};

// Enum para estratégias de limpeza de memória
UENUM(BlueprintType)
enum class EMemoryCleanupStrategy : uint8
{
    Conservative    UMETA(DisplayName = "Conservative"),
    Balanced        UMETA(DisplayName = "Balanced"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Emergency       UMETA(DisplayName = "Emergency")
};

// Estrutura para estatísticas de memória
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FMemoryStats
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TotalPhysicalGB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsedPhysicalGB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AvailablePhysicalGB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float UsagePercentage;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float GameMemoryGB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    int32 TrackedObjects;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float TextureMemoryGB;

    UPROPERTY(BlueprintReadOnly, Category = "Memory")
    float AudioMemoryGB;

    FMemoryStats()
        : TotalPhysicalGB(0.0f)
        , UsedPhysicalGB(0.0f)
        , AvailablePhysicalGB(0.0f)
        , UsagePercentage(0.0f)
        , GameMemoryGB(0.0f)
        , TrackedObjects(0)
        , TextureMemoryGB(0.0f)
        , AudioMemoryGB(0.0f)
    {}
};

// Delegate para notificações de memória
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryWarning, float, UsagePercentage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMemoryCleanup, int32, ObjectsFreed);

class TRANSPERSONALGAME_API UMemoryManager : public UObject
{
    GENERATED_BODY()

public:
    UMemoryManager();

    // Inicialização
    void Initialize();
    void Shutdown();

    // Atualização por frame
    void UpdateMemoryTracking(float DeltaTime);

    // Registro de objetos
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void RegisterObject(UObject* Object, int64 EstimatedSize, bool bIsCritical = false);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void UnregisterObject(UObject* Object);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void UpdateObjectAccess(UObject* Object);

    // Limpeza de memória
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    int32 ForceGarbageCollection();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    int32 CleanupUnusedObjects(EMemoryCleanupStrategy Strategy = EMemoryCleanupStrategy::Balanced);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void SetAutoCleanupEnabled(bool bEnabled);

    // Monitoramento
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    FMemoryStats GetMemoryStats() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    float GetMemoryUsagePercentage() const;

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    bool IsMemoryPressureHigh() const;

    // Configurações
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void SetMemoryWarningThreshold(float Threshold);

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void SetCleanupInterval(float IntervalSeconds);

    // Texture streaming
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void OptimizeTextureStreaming();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void SetTextureStreamingPool(int32 PoolSizeMB);

    // Audio memory
    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void CleanupAudioCache();

    UFUNCTION(BlueprintCallable, Category = "Memory Management")
    void SetAudioMemoryLimit(int32 LimitMB);

    // Eventos
    UPROPERTY(BlueprintAssignable, Category = "Memory Management")
    FOnMemoryWarning OnMemoryWarning;

    UPROPERTY(BlueprintAssignable, Category = "Memory Management")
    FOnMemoryCleanup OnMemoryCleanup;

    // Configurações
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryWarningThreshold = 0.8f; // 80%

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float MemoryCriticalThreshold = 0.9f; // 90%

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float CleanupInterval = 30.0f; // 30 segundos

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    float ObjectTimeoutThreshold = 300.0f; // 5 minutos

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    bool bAutoCleanupEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Memory Settings")
    int32 MaxTrackedObjects = 10000;

private:
    // Tracking de objetos
    TMap<TWeakObjectPtr<UObject>, FMemoryObjectInfo> TrackedObjects;
    TQueue<TWeakObjectPtr<UObject>> CleanupQueue;

    // Estado interno
    bool bInitialized;
    float LastCleanupTime;
    float LastMemoryCheckTime;
    float MemoryCheckInterval = 5.0f;
    bool bMemoryWarningActive;

    // Cache de estatísticas
    mutable FMemoryStats CachedStats;
    mutable float LastStatsUpdateTime;
    float StatsUpdateInterval = 1.0f;

    // Métodos internos
    void UpdateMemoryStats() const;
    void CheckMemoryPressure();
    void PerformAutoCleanup();
    int32 CleanupObjectsByStrategy(EMemoryCleanupStrategy Strategy);
    bool ShouldCleanupObject(const FMemoryObjectInfo& ObjectInfo, EMemoryCleanupStrategy Strategy) const;
    void RemoveInvalidObjects();
    float GetObjectPriority(const FMemoryObjectInfo& ObjectInfo) const;
    
    // Texture streaming helpers
    void UpdateTextureStreaming();
    void ReduceTextureQuality();
    void RestoreTextureQuality();
    
    // Audio memory helpers
    void UpdateAudioMemory();
    void UnloadUnusedAudio();
    
    // Platform-specific memory functions
    void PlatformSpecificCleanup();
    int64 GetGameMemoryUsage() const;
    int64 GetTextureMemoryUsage() const;
    int64 GetAudioMemoryUsage() const;
};
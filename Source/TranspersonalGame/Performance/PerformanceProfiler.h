// PerformanceProfiler.h
// Sistema de profiling de performance em tempo real para o jogo transpessoal

#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "HAL/Platform.h"
#include "Containers/Map.h"
#include "Containers/Array.h"

DECLARE_LOG_CATEGORY_EXTERN(LogTranspersonalPerformance, Log, All);

// Estrutura para métricas de performance
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FPerformanceMetrics
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FrameTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float FPS;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GameThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float RenderThreadTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float GPUTime;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 DrawCalls;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 Triangles;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float MemoryUsage;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    int32 ActiveActors;

    UPROPERTY(BlueprintReadOnly, Category = "Performance")
    float PhysicsTime;

    FPerformanceMetrics()
        : FrameTime(0.0f)
        , FPS(0.0f)
        , GameThreadTime(0.0f)
        , RenderThreadTime(0.0f)
        , GPUTime(0.0f)
        , DrawCalls(0)
        , Triangles(0)
        , MemoryUsage(0.0f)
        , ActiveActors(0)
        , PhysicsTime(0.0f)
    {}
};

// Enum para níveis de qualidade de performance
UENUM(BlueprintType)
enum class EPerformanceLevel : uint8
{
    Low         UMETA(DisplayName = "Low"),
    Medium      UMETA(DisplayName = "Medium"),
    High        UMETA(DisplayName = "High"),
    Ultra       UMETA(DisplayName = "Ultra"),
    Auto        UMETA(DisplayName = "Auto")
};

// Delegate para notificações de mudança de performance
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPerformanceLevelChanged, EPerformanceLevel, NewLevel);

class TRANSPERSONALGAME_API UPerformanceProfiler : public UObject
{
    GENERATED_BODY()

public:
    UPerformanceProfiler();

    // Inicialização e cleanup
    void Initialize();
    void Shutdown();

    // Atualização por frame
    void UpdateMetrics(float DeltaTime);

    // Getters para métricas
    UFUNCTION(BlueprintCallable, Category = "Performance")
    FPerformanceMetrics GetCurrentMetrics() const { return CurrentMetrics; }

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFrameTime() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    float GetAverageFPS() const;

    // Sistema de profiling automático
    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StartProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void StopProfiling();

    UFUNCTION(BlueprintCallable, Category = "Performance")
    bool IsProfilingActive() const { return bIsProfilingActive; }

    // Detecção automática de nível de performance
    UFUNCTION(BlueprintCallable, Category = "Performance")
    EPerformanceLevel GetRecommendedPerformanceLevel() const;

    UFUNCTION(BlueprintCallable, Category = "Performance")
    void SetAutoPerformanceAdjustment(bool bEnabled);

    // Eventos
    UPROPERTY(BlueprintAssignable, Category = "Performance")
    FOnPerformanceLevelChanged OnPerformanceLevelChanged;

    // Configurações de thresholds
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float TargetFPS = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float LowPerformanceThreshold = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float HighPerformanceThreshold = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Thresholds")
    float MemoryWarningThreshold = 0.8f; // 80% da memória disponível

private:
    // Métricas atuais
    FPerformanceMetrics CurrentMetrics;

    // Histórico de métricas para médias
    TArray<float> FrameTimeHistory;
    TArray<float> FPSHistory;
    static const int32 MaxHistorySize = 120; // 2 segundos a 60 FPS

    // Estado do profiler
    bool bIsProfilingActive;
    bool bAutoPerformanceAdjustment;
    float ProfilingStartTime;

    // Nível de performance atual
    EPerformanceLevel CurrentPerformanceLevel;
    float LastPerformanceCheckTime;
    float PerformanceCheckInterval = 2.0f;

    // Métodos internos
    void CollectFrameMetrics(float DeltaTime);
    void CollectRenderMetrics();
    void CollectMemoryMetrics();
    void CollectPhysicsMetrics();
    void UpdatePerformanceLevel();
    void AddToHistory(TArray<float>& History, float Value);
    float GetHistoryAverage(const TArray<float>& History) const;

    // Detecção de hardware
    void DetectHardwareCapabilities();
    bool bHardwareDetected;
    int32 EstimatedGPUPerformance; // 1-10 scale
    int32 EstimatedCPUPerformance; // 1-10 scale
    float AvailableMemoryGB;
};